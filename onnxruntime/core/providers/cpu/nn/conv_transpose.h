/**
 * Copyright (c) 2016-present, Facebook, Inc.
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
/* Modifications Copyright (c) Microsoft. */

#pragma once

#include "core/framework/op_kernel.h"
#include "core/providers/cpu/nn/conv_transpose_attributes.h"

namespace onnxruntime {

template <typename T>
class ConvTranspose : public OpKernel {
 public:
  ConvTranspose(const OpKernelInfo& info) : OpKernel(info), conv_transpose_attrs_(info) {}

  Status PrePack(const Tensor& tensor, int input_idx, AllocatorPtr alloc,
                 bool save_prepacked_initializers,
                 /*out*/ bool& is_packed,
                 /*out*/ PrePackedWeights* prepacked_weights) override;

  Status UseSharedPrePackedBuffers(std::vector<BufferUniquePtr>& prepacked_buffers,
                                   int input_idx,
                                   /*out*/ bool& used_shared_buffers) override;

  std::optional<Tensor> GetPrePackTensor(int /*input_index*/) override;

  Status SetPrePackTensor(int input_idx, const Tensor& pre_packed_tensor) override;

  Status Compute(OpKernelContext* context) const override;

 protected:
  Status DoConvTranspose(OpKernelContext* context, bool dynamic_padding) const;

 private:
  ConvTransposeAttributes conv_transpose_attrs_;

  // for pre-packing usage
  TensorShape filter_shape_;
  IAllocatorUniquePtr<void> transposed_filter_;
  // below packed_buffer and packed_tensor_ used to unpack TensorShape and packed buffer from
  // prepacked tensor read from external data file
  IAllocatorUniquePtr<void> packed_buffer_;
  std::optional<Tensor> packed_tensor_{std::nullopt};
};

}  // namespace onnxruntime
