// Copyright (C) Intel Corporation
// Licensed under the MIT License

#pragma once

#include <memory>
#include "core/session/onnxruntime_cxx_api.h"
#include "onnx_ctx_model_helper.h"

namespace onnxruntime {
namespace openvino_ep {

class IBackend {
 public:
  virtual void Infer(OrtKernelContext* context) = 0;
  virtual ov::CompiledModel& GetOVCompiledModel() = 0;
};

class BackendFactory {
 public:
  static std::shared_ptr<IBackend>
  MakeBackend(void* model_proto,
              size_t model_proto_len,
              GlobalContext& global_context,
              const SubGraphContext& subgraph_context,
              EPCtxHandler& ctx_handle);
};

}  // namespace openvino_ep
}  // namespace onnxruntime