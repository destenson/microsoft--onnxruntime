# -------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation.  All rights reserved.
# Licensed under the MIT License.
# --------------------------------------------------------------------------

"""
ONNX Model Optimizer for Stable Diffusion
"""

import gc
import logging
import os
import shutil
import tempfile
from pathlib import Path

import onnx
import psutil
import torch
from packaging import version

from onnxruntime.transformers.fusion_options import FusionOptions
from onnxruntime.transformers.onnx_model_clip import ClipOnnxModel
from onnxruntime.transformers.onnx_model_unet import UnetOnnxModel
from onnxruntime.transformers.onnx_model_vae import VaeOnnxModel
from onnxruntime.transformers.optimizer import optimize_by_onnxruntime

logger = logging.getLogger(__name__)


def memory_usage():
    gb = float(1024**3)
    free, total = torch.cuda.mem_get_info()
    m = psutil.virtual_memory()
    logger.info(
        "free gpu memory %.1f out of %.1f GB; free cpu memory %.1f out of %.1f GB",
        free / gb,
        total / gb,
        m.free / gb,
        m.total / gb,
    )


class OrtStableDiffusionOptimizer:
    def __init__(self, model_type: str):
        assert model_type in ["vae", "unet", "clip"]
        self.model_type = model_type
        self.model_type_class_mapping = {
            "unet": UnetOnnxModel,
            "vae": VaeOnnxModel,
            "clip": ClipOnnxModel,
        }

    def _optimize_by_ort(self, onnx_model, use_external_data_format, tmp_dir):
        # Save to a temporary file so that we can load it with Onnx Runtime.
        logger.info("Saving a temporary model to run OnnxRuntime graph optimizations...")
        tmp_model_path = Path(tmp_dir) / "model.onnx"
        onnx_model.save_model_to_file(str(tmp_model_path), use_external_data_format=use_external_data_format)

        del onnx_model
        gc.collect()

        ort_optimized_model_path = Path(tmp_dir) / "optimized.onnx"
        optimize_by_onnxruntime(
            str(tmp_model_path),
            use_gpu=True,
            optimized_model_path=str(ort_optimized_model_path),
            save_as_external_data=use_external_data_format,
            external_data_filename="optimized.onnx.data",
        )
        model = onnx.load(str(ort_optimized_model_path), load_external_data=True)
        return self.model_type_class_mapping[self.model_type](model)

    def optimize_by_ort(self, onnx_model, use_external_data_format=False, tmp_dir=None):
        # Use this step to see the final graph that executed by Onnx Runtime.
        if tmp_dir is None:
            with tempfile.TemporaryDirectory() as temp_dir:
                return self._optimize_by_ort(onnx_model, use_external_data_format, temp_dir)
        else:
            os.makedirs(tmp_dir, exist_ok=True)
            model = self._optimize_by_ort(onnx_model, use_external_data_format, tmp_dir)
            shutil.rmtree(tmp_dir)
            return model

    def optimize(
        self,
        input_fp32_onnx_path,
        optimized_onnx_path,
        float16=True,
        keep_io_types=False,
        fp32_op_list=None,
        keep_outputs=None,
        optimize_by_ort=True,
        optimize_by_fusion=True,
        final_target_float16=True,
        tmp_dir=None,
        use_external_data=None,
    ):
        """
        Optimize onnx model using ONNX Runtime transformers optimizer.
        The order of optimizations: Fusion (optional) -> ORT (optional) -> FP16 conversion (optional).
        """
        logger.info(f"Optimize {input_fp32_onnx_path}...")

        memory_usage()

        model = onnx.load_model(input_fp32_onnx_path, load_external_data=True)
        m = self.model_type_class_mapping[self.model_type](model)

        if optimize_by_fusion:
            fusion_options = FusionOptions(self.model_type)

            # It is allowed float16=False and final_target_float16=True, for using fp32 as intermediate optimization step.
            # For rare fp32 use case, we can disable packed kv/qkv since there is no fp32 TRT fused attention kernel.
            if self.model_type in ["unet"] and not final_target_float16:
                fusion_options.enable_packed_kv = False
                fusion_options.enable_packed_qkv = False

            m.optimize(fusion_options)
            m.topological_sort()
            m.model.producer_name = "onnxruntime.transformers"

        if keep_outputs:
            m.prune_graph(outputs=keep_outputs)

        model_size = m.model.ByteSize()
        use_external_data_format = use_external_data or model_size <= 0 or model_size >= onnx.checker.MAXIMUM_PROTOBUF
        # Note that ORT < 1.16 could not save model larger than 2GB.
        # This step is is optional since it has no impact on inference latency.
        # The optimized model is not portable. It could only run in the same execution provider (CUDA EP in this case).
        # When the model has been optimized by onnxruntime, we can disable optimization in SessionOption
        # to save session creation time. Another benefit is to inspect the final graph for developing purpose.
        from onnxruntime import __version__ as ort_version

        if optimize_by_ort and (version.parse(ort_version) >= version.parse("1.16.0") or not use_external_data_format):
            m = self.optimize_by_ort(m, use_external_data_format=use_external_data_format, tmp_dir=tmp_dir)

        if float16:
            logger.info("Convert to float16 ...")
            m.convert_float_to_float16(
                keep_io_types=keep_io_types,
                op_block_list=fp32_op_list,
            )

        m.get_operator_statistics()
        m.get_fused_operator_statistics()
        m.save_model_to_file(optimized_onnx_path, use_external_data_format=use_external_data_format)
        logger.info(f"{self.model_type} is optimized: {optimized_onnx_path}")

    def optimize_step1(
        self,
        input_fp32_onnx_path,
        fusion_onnx_path,
        keep_outputs=None,
        final_target_float16=True,
        use_external_data=True,
        float16=True,
        keep_io_types=False,
        fp32_op_list=None,
    ):
        """
        Optimize model using the following order: Fusion -> FP16 conversion (optional) -> ORT (in step2).
        We move the ORT optimization to step 2 since we can use GC to collect memory after this step.
        """
        logger.info(f"Optimize {input_fp32_onnx_path}...")

        memory_usage()

        model = onnx.load_model(input_fp32_onnx_path, load_external_data=True)
        m = self.model_type_class_mapping[self.model_type](model)

        fusion_options = FusionOptions(self.model_type)

        # It is allowed float16=False and final_target_float16=True, for using fp32 as intermediate optimization step.
        # For rare fp32 use case, we can disable packed kv/qkv since there is no fp32 TRT fused attention kernel.
        if self.model_type in ["unet"] and not final_target_float16:
            fusion_options.enable_packed_kv = False
            fusion_options.enable_packed_qkv = False

        m.optimize(fusion_options)
        m.topological_sort()
        m.model.producer_name = "onnxruntime.transformers"

        if keep_outputs:
            m.prune_graph(outputs=keep_outputs)

        if float16:
            m.convert_float_to_float16(
                keep_io_types=keep_io_types,
                op_block_list=fp32_op_list,
            )

        m.save_model_to_file(fusion_onnx_path, use_external_data_format=use_external_data)

        m.get_operator_statistics()
        m.get_fused_operator_statistics()

        logger.info(f"{self.model_type} is optimized by fusion: {fusion_onnx_path}")

    def optimize_step2(self, fusion_onnx_path, optimized_onnx_path, use_external_data):
        optimize_by_onnxruntime(
            str(fusion_onnx_path),
            use_gpu=True,
            optimized_model_path=str(optimized_onnx_path),
            save_as_external_data=use_external_data,
            external_data_filename=Path(optimized_onnx_path).name + "data",
        )

        logger.info(f"{self.model_type} is optimized: {optimized_onnx_path}")
