// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/providers/qnn/builder/qnn_context_mem_handle_manager.h"

#include "HTP/QnnHtpMem.h"

#include "core/common/common.h"
#include "core/providers/qnn/builder/qnn_def.h"
#include "core/providers/qnn/builder/qnn_utils.h"
#include "core/providers/qnn/qnn_allocator.h"

namespace onnxruntime::qnn {

QnnContextMemHandleManager::QnnContextMemHandleManager(const QNN_INTERFACE_VER_TYPE& qnn_interface,
                                                       Qnn_ContextHandle_t context,
                                                       const logging::Logger& logger)
    : qnn_interface_{qnn_interface},
      context_{context},
      logger_{logger} {
}

QnnContextMemHandleManager::~QnnContextMemHandleManager() {
  Clear();
}

Status QnnContextMemHandleManager::GetOrRegister(void* shared_memory_address, const Qnn_Tensor_t& qnn_tensor,
                                                 Qnn_MemHandle_t& qnn_mem_handle, bool& did_register) {
  const auto qnn_tensor_rank = GetQnnTensorRank(qnn_tensor);
  auto* const qnn_tensor_dims = GetQnnTensorDims(qnn_tensor);
  const auto qnn_tensor_data_type = GetQnnTensorDataType(qnn_tensor);

  const size_t qnn_tensor_data_size =
      utils::GetQnnTensorDataSize(gsl::span{qnn_tensor_dims, size_t{qnn_tensor_rank}}, qnn_tensor_data_type);

  {
    std::scoped_lock g{mem_handles_mutex_};

    // find existing mem handle
    if (const auto mem_handles_it = mem_handles_.find(shared_memory_address);
        mem_handles_it != mem_handles_.end()) {
      const auto& mem_handle_record = mem_handles_it->second;

      // check that actual tensor size is less than or equal to registered tensor size
      ORT_RETURN_IF_NOT(qnn_tensor_data_size <= mem_handle_record.registered_tensor_data_size,
                        "Actual tensor data size (", qnn_tensor_data_size,
                        ") is larger than registered tensor data size (", mem_handle_record.registered_tensor_data_size,
                        ").");

      qnn_mem_handle = mem_handle_record.mem_handle.get();
      did_register = false;
      return Status::OK();
    }

    // register a new mem handle
    HtpSharedMemoryAllocator::SharedMemoryInfo shared_memory_info{};
    ORT_RETURN_IF_ERROR(HtpSharedMemoryAllocator::GetAllocationSharedMemoryInfo(shared_memory_address,
                                                                                shared_memory_info));

    Qnn_MemDescriptor_t mem_descriptor{};
    mem_descriptor.memShape.dimSize = qnn_tensor_dims;
    mem_descriptor.memShape.numDim = qnn_tensor_rank;
    mem_descriptor.memShape.shapeConfig = nullptr;
    mem_descriptor.dataType = qnn_tensor_data_type;
    mem_descriptor.memType = QNN_MEM_TYPE_CUSTOM;

    QnnMemHtp_Descriptor_t htp_mem_descriptor{};
    htp_mem_descriptor.type = QNN_HTP_MEM_SHARED_BUFFER;
    htp_mem_descriptor.size = shared_memory_info.total_size;
    htp_mem_descriptor.sharedBufferConfig.fd = shared_memory_info.fd;
    htp_mem_descriptor.sharedBufferConfig.offset = shared_memory_info.offset;

    mem_descriptor.customInfo = &htp_mem_descriptor;

    LOGS(logger_, VERBOSE) << "Registering QNN mem handle for context: " << context_
                           << ", shared memory (address: " << shared_memory_address
                           << ", offset: " << shared_memory_info.offset
                           << ", fd: " << shared_memory_info.fd
                           << ")";

    Qnn_MemHandle_t raw_mem_handle{};
    const auto register_result = qnn_interface_.memRegister(context_, &mem_descriptor, 1, &raw_mem_handle);
    ORT_RETURN_IF_NOT(register_result == QNN_SUCCESS,
                      "qnn_interface.memRegister() failed: ", register_result);  // TODO get error message

    LOGS(logger_, VERBOSE) << "Registered QNN mem handle. mem_handle: " << raw_mem_handle;

    const auto unregister_mem_handle = [this](Qnn_MemHandle_t raw_mem_handle) {
      LOGS(logger_, VERBOSE) << "Unregistering QNN mem handle. mem_handle: " << raw_mem_handle;

      const auto unregister_result = qnn_interface_.memDeRegister(&raw_mem_handle, 1);
      if (unregister_result != QNN_SUCCESS) {
        LOGS(logger_, ERROR) << "qnn_interface.memDeRegister() failed: " << unregister_result;
        return;
      }
    };

    UniqueQnnMemHandle mem_handle(raw_mem_handle, unregister_mem_handle);
    MemHandleRecord mem_handle_record{qnn_tensor_data_size, std::move(mem_handle)};
    mem_handles_.emplace(shared_memory_address, std::move(mem_handle_record));

    qnn_mem_handle = raw_mem_handle;
    did_register = true;
    return Status::OK();
  }
}

Status QnnContextMemHandleManager::Unregister(void* shared_memory_address) {
  std::scoped_lock g{mem_handles_mutex_};

  auto mem_handles_it = mem_handles_.find(shared_memory_address);
  ORT_RETURN_IF_NOT(mem_handles_it != mem_handles_.end(),
                    "No mem handle found for address (", shared_memory_address, ").");

  mem_handles_.erase(mem_handles_it);

  return Status::OK();
}

void QnnContextMemHandleManager::Clear() {
  std::scoped_lock g{mem_handles_mutex_};
  mem_handles_.clear();
}

}  // namespace onnxruntime::qnn
