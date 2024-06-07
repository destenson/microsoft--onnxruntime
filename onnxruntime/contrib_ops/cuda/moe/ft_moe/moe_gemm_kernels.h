/*
 * Copyright (c) 2020-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "contrib_ops/cuda/moe/cutlass_extensions/gemm_configs.h"
#include <cuda_runtime_api.h>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace ort_fastertransformer {

struct HopperGroupedGemmInput
{
    template <class Tag>
    using TransposeLayoutTag = std::conditional_t<std::is_same_v<Tag, cutlass::layout::RowMajor>,
        cutlass::layout::ColumnMajor, cutlass::layout::RowMajor>;
    static_assert(std::is_same_v<cutlass::layout::RowMajor, TransposeLayoutTag<cutlass::layout::ColumnMajor>>);
    static_assert(std::is_same_v<cutlass::layout::ColumnMajor, TransposeLayoutTag<cutlass::layout::RowMajor>>);

    // Layout for A and B is transposed and then swapped in the implementation
    // This uses B^T * A^T = (A * B)^T to get a better layout for the GEMM
    using LayoutA = TransposeLayoutTag<cutlass::layout::RowMajor>;    // Layout type for A matrix operand
    using LayoutB = TransposeLayoutTag<cutlass::layout::ColumnMajor>; // Layout type for B matrix operand
    using LayoutC = TransposeLayoutTag<cutlass::layout::RowMajor>;    // Layout type for C matrix operand
    using LayoutD = TransposeLayoutTag<cutlass::layout::RowMajor>;    // Layout type for D matrix operand

    using StrideA
        = std::remove_pointer_t<cutlass::detail::TagToStrideB_t<LayoutA*>>; // Use B because they will be swapped
    using StrideB
        = std::remove_pointer_t<cutlass::detail::TagToStrideA_t<LayoutB*>>; // Use A because they will be swapped
    using StrideC = std::remove_pointer_t<cutlass::detail::TagToStrideC_t<LayoutC*>>;
    using StrideD = std::remove_pointer_t<cutlass::detail::TagToStrideC_t<LayoutD*>>;

    template <class T>
    constexpr static bool IsFP8_v = std::is_same_v<T, __nv_fp8_e4m3> || std::is_same_v<T, __nv_fp8_e5m2>;
    template <class T>
    using OutputTypeAdaptor_t = std::conditional_t<IsFP8_v<T>, float, T>;

    using ProblemShape = cutlass::gemm::GroupProblemShape<cute::Shape<int64_t, int64_t, int64_t>>;

    ProblemShape shape_info{};
    StrideA* stride_a = nullptr;
    StrideB* stride_b = nullptr;
    StrideC* stride_c = nullptr;
    StrideD* stride_d = nullptr;

    void const** ptr_a = nullptr;
    void const** ptr_b = nullptr;
    void const** ptr_c = nullptr;
    void** ptr_d = nullptr;

    float const** alpha_scale_ptr_array = nullptr;

    uint8_t* gemm_workspace = nullptr;
    size_t gemm_workspace_size = 0;

    static auto workspaceBuffers(int num_experts)
    {
        size_t problem_shape_size = sizeof(ProblemShape::UnderlyingProblemShape) * num_experts;
        size_t stride_a_size = sizeof(StrideA) * num_experts;
        size_t stride_b_size = sizeof(StrideB) * num_experts;
        size_t stride_c_size = sizeof(StrideC) * num_experts;
        size_t stride_d_size = sizeof(StrideD) * num_experts;

        size_t ptr_buf_size = sizeof(void*) * num_experts;
        size_t scale_buf_size = sizeof(float**) * num_experts;

        return std::array{problem_shape_size, stride_a_size, stride_b_size, stride_c_size, stride_d_size, ptr_buf_size,
            ptr_buf_size, ptr_buf_size, ptr_buf_size, scale_buf_size};
    }

    static size_t workspaceSize(int num_experts)
    {
        auto buffers = workspaceBuffers(num_experts);
        return tensorrt_llm::common::calculateTotalWorkspaceSize(buffers.data(), buffers.size());
    }

    void configureWorkspace(int8_t* start_ptr, int num_experts, void* gemm_workspace, size_t gemm_workspace_size)
    {
        auto buffers = workspaceBuffers(num_experts);
        std::array<int8_t*, 10> pointers{};
        TLLM_CHECK_WITH_INFO(pointers.size() == buffers.size(), "Mismatching workspace size and number of buffers");
        for (int i = 0; i < buffers.size(); i++)
        {
            pointers[i] = start_ptr;
            start_ptr = tensorrt_llm::common::nextWorkspacePtr(start_ptr, buffers[i]);
        }

        shape_info.num_groups = num_experts;
        shape_info.problem_shapes = reinterpret_cast<ProblemShape::UnderlyingProblemShape*>(pointers[0]);
        shape_info.host_problem_shapes = nullptr;
        stride_a = reinterpret_cast<StrideA*>(pointers[1]);
        stride_b = reinterpret_cast<StrideB*>(pointers[2]);
        stride_c = reinterpret_cast<StrideC*>(pointers[3]);
        stride_d = reinterpret_cast<StrideD*>(pointers[4]);

        ptr_a = reinterpret_cast<void const**>(pointers[5]);
        ptr_b = reinterpret_cast<void const**>(pointers[6]);
        ptr_c = reinterpret_cast<void const**>(pointers[7]);
        ptr_d = reinterpret_cast<void**>(pointers[8]);

        alpha_scale_ptr_array = reinterpret_cast<float const**>(pointers[9]);

        this->gemm_workspace = reinterpret_cast<uint8_t*>(gemm_workspace);
        this->gemm_workspace_size = gemm_workspace_size;
    }

    bool isValid() const
    {
        return stride_a != nullptr && ptr_a != nullptr;
    }
};

struct MoEGemmConfigMap {
  using MoEGemmConfigMapT = std::unordered_map<int64_t, CutlassGemmConfig>;

  MoEGemmConfigMapT map;
  std::mutex mutex;

  void Insert(int64_t key, CutlassGemmConfig config) {
    std::lock_guard<std::mutex> lock(mutex);
    map[key] = config;
  }

  bool Contains(int64_t key) {
    std::lock_guard<std::mutex> lock(mutex);
    return map.find(key) != map.end();
  }

  CutlassGemmConfig Get(int64_t key) {
    std::lock_guard<std::mutex> lock(mutex);
    return map[key];
  }
};

enum class ActivationType { Gelu,
                            Relu,
                            Silu,
                            GeGLU,
                            ReGLU,
                            SiGLU,
                            Identity,
                            InvalidType };

template <typename T, /*The type used for activations/scales/compute*/
          typename WeightType /* The type for the MoE weights */>
class MoeGemmRunner {
 public:
  MoeGemmRunner();

  void initialize(int sm);

  void moe_gemm_bias_act(const T* A, const WeightType* B, const T* weight_scales, const T* biases, T* C,
                         int64_t* total_rows_before_expert, HopperGroupedGemmInput layout_info, int64_t total_rows,
                         int64_t gemm_n, int64_t gemm_k, int num_experts, ActivationType activation_type,
                         cudaStream_t stream);

  void moe_gemm(const T* A, const WeightType* B, const T* weight_scales, const T* biases, T* C,
                int64_t* total_rows_before_expert, HopperGroupedGemmInput layout_info, int64_t total_rows,
                int64_t gemm_n, int64_t gemm_k, int num_experts, cudaStream_t stream);

  static MoEGemmConfigMap& GetGemmConfigMap() {
    static MoEGemmConfigMap gFactory;
    return gFactory;
  }

 private:
  template <typename EpilogueTag>
  void dispatch_to_arch(const T* A, const WeightType* B, const T* weight_scales, const T* biases, T* C,
                        int64_t* total_rows_before_expert, HopperGroupedGemmInput layout_info, int64_t total_rows,
                        int64_t gemm_n, int64_t gemm_k, int num_experts, CutlassGemmConfig gemm_config,
                        cudaStream_t stream, int* occupancy = nullptr);

  template <typename EpilogueTag>
  void profile_gemm(const T* A, const WeightType* B, const T* weight_scales, const T* biases, T* C,
                    int64_t* total_rows_before_expert, HopperGroupedGemmInput layout_info, int64_t total_rows,
                    int64_t gemm_n, int64_t gemm_k, int num_experts, cudaStream_t stream, int64_t key);

  template <typename EpilogueTag>
  void run_gemm(const T* A, const WeightType* B, const T* weight_scales, const T* biases, T* C,
                int64_t* total_rows_before_expert, HopperGroupedGemmInput layout_info, int64_t total_rows,
                int64_t gemm_n, int64_t gemm_k, int num_experts, cudaStream_t stream);

 private:
  int sm_;
  int multi_processor_count_;
};

}  // namespace ort_fastertransformer
