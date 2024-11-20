// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <functional>
#include <numeric>
#include <string>
#include <vector>
#include <map>

#include "core/common/common.h"
#include "core/framework/data_types.h"
#include "qnn_utils.h"
#include "core/providers/qnn/builder/qnn_def.h"

namespace onnxruntime {
namespace qnn {
namespace utils {

size_t GetElementSizeByType(const Qnn_DataType_t& data_type) {
  const static std::unordered_map<Qnn_DataType_t, size_t> data_type_to_size = {
      {QNN_DATATYPE_INT_8, 1},
      {QNN_DATATYPE_INT_16, 2},
      {QNN_DATATYPE_INT_32, 4},
      {QNN_DATATYPE_INT_64, 8},
      {QNN_DATATYPE_UINT_8, 1},
      {QNN_DATATYPE_UINT_16, 2},
      {QNN_DATATYPE_UINT_32, 4},
      {QNN_DATATYPE_UINT_64, 8},
      {QNN_DATATYPE_FLOAT_16, 2},
      {QNN_DATATYPE_FLOAT_32, 4},
      {QNN_DATATYPE_BOOL_8, 1},
      {QNN_DATATYPE_SFIXED_POINT_8, 1},
      {QNN_DATATYPE_SFIXED_POINT_16, 2},
      {QNN_DATATYPE_SFIXED_POINT_32, 4},
      {QNN_DATATYPE_UFIXED_POINT_8, 1},
      {QNN_DATATYPE_UFIXED_POINT_16, 2},
      {QNN_DATATYPE_UFIXED_POINT_32, 4},
  };

  auto pos = data_type_to_size.find(data_type);
  ORT_ENFORCE(pos != data_type_to_size.end(), "Unknown QNN data type", data_type);
  return pos->second;
}
size_t GetElementSizeByType(ONNXTensorElementDataType elem_type) {
  const static std::unordered_map<ONNXTensorElementDataType, size_t> elem_type_to_size = {
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_INT4, sizeof(Int4x2)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT4, sizeof(UInt4x2)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8, sizeof(int8_t)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16, sizeof(int16_t)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32, sizeof(int32_t)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64, sizeof(int64_t)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8, sizeof(uint8_t)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16, sizeof(uint16_t)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32, sizeof(uint32_t)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64, sizeof(uint64_t)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16, 2},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, sizeof(float)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE, sizeof(double)},
      {ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL, sizeof(bool)}};

  auto pos = elem_type_to_size.find(elem_type);
  ORT_ENFORCE(pos != elem_type_to_size.end(), "Unknown element type", elem_type);
  return pos->second;
}

std::ostream& operator<<(std::ostream& out, const Qnn_Scalar_t& scalar) {
  switch (scalar.dataType) {
    case QNN_DATATYPE_INT_8:
      out << static_cast<int32_t>(scalar.int8Value);
      break;
    case QNN_DATATYPE_INT_16:
      out << scalar.int16Value;
      break;
    case QNN_DATATYPE_INT_32:
      out << scalar.int32Value;
      break;
    case QNN_DATATYPE_INT_64:
      out << "int64_t is not supported";
      break;
    case QNN_DATATYPE_UINT_8:
      out << static_cast<int32_t>(scalar.uint8Value);
      break;
    case QNN_DATATYPE_UINT_16:
      out << scalar.uint16Value;
      break;
    case QNN_DATATYPE_UINT_32:
      out << scalar.uint32Value;
      break;
    case QNN_DATATYPE_UINT_64:
      out << "uint64_t is not supported";
      break;
    case QNN_DATATYPE_FLOAT_16:
      break;
    case QNN_DATATYPE_FLOAT_32:
      out << scalar.floatValue;
      break;
    case QNN_DATATYPE_SFIXED_POINT_8:
    case QNN_DATATYPE_SFIXED_POINT_16:
    case QNN_DATATYPE_SFIXED_POINT_32:
    case QNN_DATATYPE_UFIXED_POINT_8:
    case QNN_DATATYPE_UFIXED_POINT_16:
    case QNN_DATATYPE_UFIXED_POINT_32:
      out << "usigned fixedpoint data is not supported";
      break;
    case QNN_DATATYPE_BOOL_8:
      out << static_cast<int32_t>(scalar.bool8Value);
      break;
    default:
      ORT_THROW("Unknown Qnn Data type");
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_DataType_t& data_type) {
  switch (data_type) {
    case QNN_DATATYPE_INT_8:
      out << "QNN_DATATYPE_INT_8";
      break;
    case QNN_DATATYPE_INT_16:
      out << "QNN_DATATYPE_INT_16";
      break;
    case QNN_DATATYPE_INT_32:
      out << "QNN_DATATYPE_INT_32";
      break;
    case QNN_DATATYPE_INT_64:
      out << "QNN_DATATYPE_INT_64";
      break;
    case QNN_DATATYPE_UINT_8:
      out << "QNN_DATATYPE_UINT_8";
      break;
    case QNN_DATATYPE_UINT_16:
      out << "QNN_DATATYPE_UINT_16";
      break;
    case QNN_DATATYPE_UINT_32:
      out << "QNN_DATATYPE_UINT_32";
      break;
    case QNN_DATATYPE_UINT_64:
      out << "QNN_DATATYPE_UINT_64";
      break;
    case QNN_DATATYPE_FLOAT_16:
      out << "QNN_DATATYPE_FLOAT_16";
      break;
    case QNN_DATATYPE_FLOAT_32:
      out << "QNN_DATATYPE_FLOAT_32";
      break;
    case QNN_DATATYPE_SFIXED_POINT_8:
      out << "QNN_DATATYPE_SFIXED_POINT_8";
      break;
    case QNN_DATATYPE_SFIXED_POINT_16:
      out << "QNN_DATATYPE_SFIXED_POINT_16";
      break;
    case QNN_DATATYPE_SFIXED_POINT_32:
      out << "QNN_DATATYPE_SFIXED_POINT_32";
      break;
    case QNN_DATATYPE_UFIXED_POINT_8:
      out << "QNN_DATATYPE_UFIXED_POINT_8";
      break;
    case QNN_DATATYPE_UFIXED_POINT_16:
      out << "QNN_DATATYPE_UFIXED_POINT_16";
      break;
    case QNN_DATATYPE_UFIXED_POINT_32:
      out << "QNN_DATATYPE_UFIXED_POINT_32";
      break;
    case QNN_DATATYPE_BOOL_8:
      out << "QNN_DATATYPE_BOOL_8";
      break;
    case QNN_DATATYPE_SFIXED_POINT_4:
      out << "QNN_DATATYPE_SFIXED_POINT_4";
      break;
    case QNN_DATATYPE_UFIXED_POINT_4:
      out << "QNN_DATATYPE_UFIXED_POINT_4";
      break;
    default:
      ORT_THROW("Unknown Qnn Data type");
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_Definition_t& definition) {
  switch (definition) {
    case QNN_DEFINITION_IMPL_GENERATED:
      out << "QNN_DEFINITION_IMPL_GENERATED";
      break;
    case QNN_DEFINITION_DEFINED:
      out << "QNN_DEFINITION_DEFINED";
      break;
    case QNN_DEFINITION_UNDEFINED:
      out << "QNN_DEFINITION_UNDEFINED";
      break;
    default:
      out << "Undefined";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_QuantizationEncoding_t& encoding) {
  switch (encoding) {
    case QNN_QUANTIZATION_ENCODING_SCALE_OFFSET:
      out << "QNN_QUANTIZATION_ENCODING_SCALE_OFFSET";
      break;
    case QNN_QUANTIZATION_ENCODING_AXIS_SCALE_OFFSET:
      out << "QNN_QUANTIZATION_ENCODING_AXIS_SCALE_OFFSET";
      break;
    case QNN_QUANTIZATION_ENCODING_BW_SCALE_OFFSET:
      out << "QNN_QUANTIZATION_ENCODING_BW_SCALE_OFFSET";
      break;
    case QNN_QUANTIZATION_ENCODING_BW_AXIS_SCALE_OFFSET:
      out << "QNN_QUANTIZATION_ENCODING_BW_AXIS_SCALE_OFFSET";
      break;
    case QNN_QUANTIZATION_ENCODING_UNDEFINED:
      out << "QNN_QUANTIZATION_ENCODING_UNDEFINED";
      break;
    default:
      out << "Uknown quantization encoding";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_QuantizeParams_t& quantize_params) {
  out << " encodingDefinition=" << quantize_params.encodingDefinition;
  out << " quantizationEncoding=" << quantize_params.quantizationEncoding;
  if (quantize_params.encodingDefinition == QNN_DEFINITION_IMPL_GENERATED ||
      quantize_params.encodingDefinition == QNN_DEFINITION_DEFINED) {
    if (quantize_params.quantizationEncoding == QNN_QUANTIZATION_ENCODING_SCALE_OFFSET) {
      out << " scale=" << quantize_params.scaleOffsetEncoding.scale;
      out << " offset=" << quantize_params.scaleOffsetEncoding.offset;
    } else if (quantize_params.quantizationEncoding == QNN_QUANTIZATION_ENCODING_BW_SCALE_OFFSET) {
      out << " bitwidth=" << quantize_params.bwScaleOffsetEncoding.bitwidth;
      out << " scale=" << quantize_params.bwScaleOffsetEncoding.scale;
      out << " offset=" << quantize_params.bwScaleOffsetEncoding.offset;
    } else if (quantize_params.quantizationEncoding == QNN_QUANTIZATION_ENCODING_AXIS_SCALE_OFFSET) {
      out << " axis=" << quantize_params.axisScaleOffsetEncoding.axis;
      size_t num_elems = quantize_params.axisScaleOffsetEncoding.numScaleOffsets;
      bool truncate = num_elems > 20;
      num_elems = truncate ? 20 : num_elems;
      out << " scales=(";
      for (size_t i = 0; i < num_elems; i++) {
        out << quantize_params.axisScaleOffsetEncoding.scaleOffset[i].scale << (i == num_elems - 1 ? "" : " ");
      }
      out << ") offsets=(";
      for (size_t i = 0; i < num_elems; i++) {
        out << quantize_params.axisScaleOffsetEncoding.scaleOffset[i].offset << (i == num_elems - 1 ? "" : " ");
      }
      out << (truncate ? "...)" : ")");
    } else if (quantize_params.quantizationEncoding == QNN_QUANTIZATION_ENCODING_BW_AXIS_SCALE_OFFSET) {
      out << " axis=" << quantize_params.bwAxisScaleOffsetEncoding.axis;
      out << " bw=" << quantize_params.bwAxisScaleOffsetEncoding.bitwidth;
      size_t num_elems = quantize_params.bwAxisScaleOffsetEncoding.numElements;
      bool truncate = num_elems > 20;
      num_elems = truncate ? 20 : num_elems;
      out << " scales=(";
      for (size_t i = 0; i < num_elems; i++) {
        out << quantize_params.bwAxisScaleOffsetEncoding.scales[i] << (i == num_elems - 1 ? "" : " ");
      }
      out << ") offsets=(";
      for (size_t i = 0; i < num_elems; i++) {
        out << quantize_params.bwAxisScaleOffsetEncoding.offsets[i] << (i == num_elems - 1 ? "" : " ");
      }
      out << (truncate ? "...)" : ")");
    } else {
      out << " encoding not supported.";
    }
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_TensorType_t& tensor_type) {
  switch (tensor_type) {
    case QNN_TENSOR_TYPE_APP_WRITE:
      out << "QNN_TENSOR_TYPE_APP_WRITE";
      break;
    case QNN_TENSOR_TYPE_APP_READ:
      out << "QNN_TENSOR_TYPE_APP_READ";
      break;
    case QNN_TENSOR_TYPE_APP_READWRITE:
      out << "QNN_TENSOR_TYPE_APP_READWRITE";
      break;
    case QNN_TENSOR_TYPE_NATIVE:
      out << "QNN_TENSOR_TYPE_NATIVE";
      break;
    case QNN_TENSOR_TYPE_STATIC:
      out << "QNN_TENSOR_TYPE_STATIC";
      break;
    case QNN_TENSOR_TYPE_NULL:
      out << "QNN_TENSOR_TYPE_NULL";
      break;
    default:
      out << "Unsupported type";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_TensorMemType_t& mem_type) {
  switch (mem_type) {
    case QNN_TENSORMEMTYPE_RAW:
      out << "QNN_TENSORMEMTYPE_RAW";
      break;
    case QNN_TENSORMEMTYPE_MEMHANDLE:
      out << "QNN_TENSORMEMTYPE_MEMHANDLE";
      break;
    default:
      out << "Unsupported mem type";
  }
  return out;
}
template <typename T>
std::ostream& operator<<(std::ostream& out, const Qnn_ClientBuffer_t& client_bufer) {
  T* data = reinterpret_cast<T*>(client_bufer.data);
  out << " dataSize=" << client_bufer.dataSize;
  uint32_t count = client_bufer.dataSize / sizeof(T);
  const bool truncate = count > 100;

  count = truncate ? 100 : count;  // limit to 100 data
  out << " clientBuf=(";
  for (uint32_t i = 0; i < count; i++) {
    if constexpr (sizeof(T) == 1) {
      out << static_cast<int32_t>(data[i]) << " ";
    } else {
      out << data[i] << " ";
    }
  }
  out << (truncate ? "..." : "") << ")";
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_Tensor_t& tensor) {
  out << " name=" << GetQnnTensorName(tensor);
  out << " id=" << GetQnnTensorID(tensor);
  out << " version=" << tensor.version;
  out << " type=" << GetQnnTensorType(tensor);
  out << " dataFormat=" << GetQnnTensorDataFormat(tensor);
  out << " dataType=" << GetQnnTensorDataType(tensor);
  out << " rank=" << GetQnnTensorRank(tensor);
  out << " dimensions=(";
  for (uint32_t i = 0; i < GetQnnTensorRank(tensor); i++) {
    out << GetQnnTensorDims(tensor)[i] << " ";
  }
  out << ")";
  out << " memType=" << GetQnnTensorMemType(tensor);
// TODO: the code below has compilation errors with the latest ABSL
#if 0
  if (GetQnnTensorMemType(tensor) == QNN_TENSORMEMTYPE_RAW) {
    if (GetQnnTensorDataType(tensor) == QNN_DATATYPE_FLOAT_32) {
      operator<< <float>(out, GetQnnTensorClientBuf(tensor));
    } else if (GetQnnTensorDataType(tensor) == QNN_DATATYPE_UINT_32 ||
               GetQnnTensorDataType(tensor) == QNN_DATATYPE_UFIXED_POINT_32) {
      operator<< <uint32_t>(out, GetQnnTensorClientBuf(tensor));
    } else if (GetQnnTensorDataType(tensor) == QNN_DATATYPE_INT_32 ||
               GetQnnTensorDataType(tensor) == QNN_DATATYPE_SFIXED_POINT_32) {
      operator<< <int32_t>(out, GetQnnTensorClientBuf(tensor));
    } else if (GetQnnTensorDataType(tensor) == QNN_DATATYPE_UINT_16 ||
               GetQnnTensorDataType(tensor) == QNN_DATATYPE_UFIXED_POINT_16) {
      operator<< <uint16_t>(out, GetQnnTensorClientBuf(tensor));
    } else if (GetQnnTensorDataType(tensor) == QNN_DATATYPE_INT_16 ||
               GetQnnTensorDataType(tensor) == QNN_DATATYPE_SFIXED_POINT_16) {
      operator<< <int16_t>(out, GetQnnTensorClientBuf(tensor));
    } else if (GetQnnTensorDataType(tensor) == QNN_DATATYPE_UINT_8 ||
               GetQnnTensorDataType(tensor) == QNN_DATATYPE_UFIXED_POINT_8) {
      operator<< <uint8_t>(out, GetQnnTensorClientBuf(tensor));
    } else {
      operator<< <int8_t>(out, GetQnnTensorClientBuf(tensor));
    }
  }
#endif
  out << " quantizeParams:" << GetQnnTensorQParams(tensor);
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_ParamType_t& param_type) {
  switch (param_type) {
    case QNN_PARAMTYPE_SCALAR:
      out << "QNN_PARAMTYPE_SCALAR";
      break;
    case QNN_PARAMTYPE_TENSOR:
      out << "QNN_PARAMTYPE_TENSOR";
      break;
    default:
      out << "Unknown type";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_Param_t& qnn_param) {
  out << " type=" << qnn_param.paramType;
  out << " name=" << qnn_param.name;
  if (qnn_param.paramType == QNN_PARAMTYPE_TENSOR) {
    out << qnn_param.tensorParam;
  } else {
    out << " value=" << qnn_param.scalarParam;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const QnnOpConfigWrapper& op_conf_wrapper) {
  out << "Qnn_OpConfig node name: " << op_conf_wrapper.GetOpName()
      << " package_name: " << op_conf_wrapper.GetPackageName()
      << " QNN_op_type: " << op_conf_wrapper.GetTypeName()
      << " num_of_inputs: " << op_conf_wrapper.GetInputsNum()
      << " num_of_outputs: " << op_conf_wrapper.GetOutputsNum()
      << " num_of_params: " << op_conf_wrapper.GetParamsNum();

  out << std::endl
      << " node_inputs:" << std::endl;
  for (uint32_t i = 0; i < op_conf_wrapper.GetInputsNum(); i++) {
    out << op_conf_wrapper.GetInputTensors()[i] << std::endl;
  }
  out << " node_outputs:" << std::endl;
  for (uint32_t i = 0; i < op_conf_wrapper.GetOutputsNum(); i++) {
    out << op_conf_wrapper.GetOutputTensors()[i] << std::endl;
  }
  out << " node_params:" << std::endl;
  for (uint32_t i = 0; i < op_conf_wrapper.GetParamsNum(); i++) {
    out << op_conf_wrapper.GetParams()[i] << std::endl;
  }
  return out;
}

Status GetQnnDataType(const bool is_quantized_tensor, const ONNX_NAMESPACE::TypeProto* type_proto,
                      Qnn_DataType_t& tensor_data_type) {
  if (!type_proto || !type_proto->tensor_type().has_elem_type()) {
    return ORT_MAKE_STATUS(ONNXRUNTIME, INVALID_ARGUMENT, "The tensor doesn't have elem_type.");
  }

  int32_t onnx_data_type = type_proto->tensor_type().elem_type();
  ORT_RETURN_IF_NOT(OnnxDataTypeToQnnDataType(onnx_data_type, tensor_data_type, is_quantized_tensor),
                    "Failed to map Onnx data type to Qnn data type!");

  return Status::OK();
}

const std::string& GetNodeName(const NodeUnit& node_unit) {
  const std::string& node_name = node_unit.Name();
  if (node_name.empty()) {
    return node_unit.Outputs()[0].node_arg.Name();
  }

  return node_name;
}

bool OnnxDataTypeToQnnDataType(const int32_t onnx_data_type, Qnn_DataType_t& qnn_data_type, bool is_quantized) {
  const std::unordered_map<int32_t, Qnn_DataType_t> onnx_to_qnn_data_type = {
      {ONNX_NAMESPACE::TensorProto_DataType_INT8, QNN_DATATYPE_INT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_INT16, QNN_DATATYPE_INT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_INT32, QNN_DATATYPE_INT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_INT64, QNN_DATATYPE_INT_64},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT8, QNN_DATATYPE_UINT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT16, QNN_DATATYPE_UINT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT32, QNN_DATATYPE_UINT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT64, QNN_DATATYPE_UINT_64},
      {ONNX_NAMESPACE::TensorProto_DataType_FLOAT16, QNN_DATATYPE_FLOAT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_FLOAT, QNN_DATATYPE_FLOAT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_BOOL, QNN_DATATYPE_BOOL_8},
  };

  const std::unordered_map<int32_t, Qnn_DataType_t> onnx_to_qnn_data_type_quantized = {
      {ONNX_NAMESPACE::TensorProto_DataType_INT4, QNN_DATATYPE_SFIXED_POINT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_INT8, QNN_DATATYPE_SFIXED_POINT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_INT16, QNN_DATATYPE_SFIXED_POINT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_INT32, QNN_DATATYPE_SFIXED_POINT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_INT64, QNN_DATATYPE_INT_64},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT4, QNN_DATATYPE_UFIXED_POINT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT8, QNN_DATATYPE_UFIXED_POINT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT16, QNN_DATATYPE_UFIXED_POINT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT32, QNN_DATATYPE_UFIXED_POINT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT64, QNN_DATATYPE_UINT_64},
      {ONNX_NAMESPACE::TensorProto_DataType_FLOAT16, QNN_DATATYPE_FLOAT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_FLOAT, QNN_DATATYPE_FLOAT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_BOOL, QNN_DATATYPE_BOOL_8},
  };

  const auto do_type_mapping = [](const std::unordered_map<int32_t, Qnn_DataType_t>& mapping_table,
                                  const int32_t onnx_data_type,
                                  Qnn_DataType_t& qnn_data_type) -> bool {
    auto pos = mapping_table.find(onnx_data_type);
    if (pos == mapping_table.end()) {
      return false;
    }
    qnn_data_type = pos->second;
    return true;
  };

  if (is_quantized) {
    return do_type_mapping(onnx_to_qnn_data_type_quantized, onnx_data_type, qnn_data_type);
  } else {
    return do_type_mapping(onnx_to_qnn_data_type, onnx_data_type, qnn_data_type);
  }
}

std::pair<float, float> CheckMinMax(float rmin, float rmax) {
  // Ensure a minimum range of 0.0001 (required by QNN)
  rmax = std::max(rmax, rmin + 0.0001f);

  // Both QNN and ORT require the range to include 0.0f
  rmin = std::min(rmin, 0.0f);
  rmax = std::max(rmax, 0.0f);

  return std::make_pair(rmin, rmax);
}

Status GetQuantParams(float rmin,
                      float rmax,
                      const Qnn_DataType_t qnn_data_type,
                      float& scale,
                      int32_t& zero_point,
                      bool symmetric) {
  std::tie(rmin, rmax) = CheckMinMax(rmin, rmax);
  if (symmetric) {
    float abs_max = std::max(abs(rmax), abs(rmin));
    rmax = abs_max;
    rmin = -abs_max;
  }

  double rmin_dbl = static_cast<double>(rmin);
  double rmax_dbl = static_cast<double>(rmax);
  double qmin = 0.0;
  double qmax = 0.0;
  ORT_RETURN_IF_ERROR(GetQminQmax(qnn_data_type, qmin, qmax, symmetric));

  double scale_dbl = (rmax_dbl - rmin_dbl) / (qmax - qmin);
  double initial_zero_point = 0.0;
  if (symmetric) {
    initial_zero_point = std::round(rmin_dbl + rmax_dbl) / 2;
  } else {
    initial_zero_point = qmin - (rmin_dbl / scale_dbl);
  }
  zero_point = static_cast<int32_t>(RoundHalfToEven(static_cast<float>(Saturate(qmax, qmin, initial_zero_point))));
  zero_point = -zero_point;  // Negate to match QNN quantization definition.
  scale = static_cast<float>(scale_dbl);
  return Status::OK();
}

double Dequantize(int32_t offset, float scale, const double quant_value) {
  double offset_d = static_cast<double>(offset);
  double scale_d = static_cast<double>(scale);
  return (quant_value + offset_d) * scale_d;
}

Status Quantize(const double double_value,
                const float scale,
                const int32_t zero_point,
                const Qnn_DataType_t qnn_data_type,
                int& quant_value) {
  int qmin = 0;
  int qmax = 255;
  ORT_RETURN_IF_ERROR(GetQminQmax(qnn_data_type, qmin, qmax));
  quant_value = Saturate(qmax, qmin, static_cast<int>(std::round((double_value / scale) - zero_point)));
  return Status::OK();
}

size_t ShapeSizeCalc(gsl::span<const uint32_t> shape, size_t start, size_t end) {
  size_t size = 1;
  for (size_t i = start; i < end; i++) {
    size *= shape[i];
  }
  return size;
}

Status GetDataQuantParams(gsl::span<const float> data, gsl::span<const uint32_t> shape,
                          /*out*/ gsl::span<float> scales, /*out*/ gsl::span<int32_t> offsets,
                          Qnn_DataType_t data_type, bool symmetric, std::optional<int64_t> axis) {
  const size_t num_dims = shape.size();
  const size_t num_elems = ShapeSizeCalc(shape, 0, num_dims);
  ORT_RETURN_IF_NOT(num_elems == data.size(), "Shape mismatch with data to quantize");

  size_t block_count = 1;
  size_t broadcast_dim = 1;
  size_t block_size = num_elems;

  if (axis.has_value()) {
    size_t axis_no_neg = *axis < 0 ? static_cast<size_t>(*axis) + num_dims : static_cast<size_t>(*axis);
    block_count = ShapeSizeCalc(shape, 0, axis_no_neg);
    broadcast_dim = shape[axis_no_neg];
    block_size = ShapeSizeCalc(shape, axis_no_neg + 1, num_dims);
  }

  ORT_RETURN_IF_NOT(scales.size() == broadcast_dim, "Unexpected size of scales output buffer");
  ORT_RETURN_IF_NOT(offsets.size() == broadcast_dim, "Unexpected size of offsets output buffer");

  size_t i = 0;
  for (size_t n = 0; n < block_count; n++) {
    for (size_t bd = 0; bd < broadcast_dim; bd++) {
      float rmin = std::numeric_limits<float>::max();
      float rmax = std::numeric_limits<float>::lowest();
      for (size_t j = 0; j < block_size; j++) {
        rmin = std::min(rmin, data[i]);
        rmax = std::max(rmax, data[i]);
        i++;
      }

      scales[bd] = 1.0f;
      offsets[bd] = 0;
      ORT_RETURN_IF_ERROR(GetQuantParams(rmin, rmax, data_type, scales[bd], offsets[bd], symmetric));
    }
  }

  assert(i == data.size());
  return Status::OK();
}

Status QuantizeData(gsl::span<const float> data, gsl::span<const uint32_t> shape,
                    gsl::span<const float> scales, gsl::span<const int32_t> offsets,
                    /*out*/ gsl::span<uint8_t> quant_bytes, Qnn_DataType_t data_type,
                    std::optional<int64_t> axis) {
  const size_t num_dims = shape.size();
  const size_t num_elems = ShapeSizeCalc(shape, 0, num_dims);
  ORT_RETURN_IF_NOT(num_elems == data.size(), "Shape mismatch with data to quantize");
  size_t expected_num_quant_bytes = GetElementSizeByType(data_type) * data.size();
  ORT_RETURN_IF_NOT(quant_bytes.size() == expected_num_quant_bytes,
                    "Cannot quantize data because output buffer is not the correct size");

  size_t block_count = 1;
  size_t broadcast_dim = 1;
  size_t block_size = num_elems;

  if (axis.has_value()) {
    size_t axis_no_neg = *axis < 0 ? static_cast<size_t>(*axis) + num_dims : static_cast<size_t>(*axis);
    block_count = ShapeSizeCalc(shape, 0, axis_no_neg);
    broadcast_dim = shape[axis_no_neg];
    block_size = ShapeSizeCalc(shape, axis_no_neg + 1, num_dims);
  }

  ORT_RETURN_IF_NOT(scales.size() == broadcast_dim, "Unexpected size of scales output buffer");
  ORT_RETURN_IF_NOT(offsets.size() == broadcast_dim, "Unexpected size of offsets output buffer");

  size_t i = 0;
  for (size_t n = 0; n < block_count; n++) {
    for (size_t bd = 0; bd < broadcast_dim; bd++) {
      switch (data_type) {
        case QNN_DATATYPE_SFIXED_POINT_8: {
          int8_t* output = reinterpret_cast<int8_t*>(quant_bytes.data());
          ParQuantizeLinearStd(&data[i], &output[i], block_size, scales[bd], static_cast<int8_t>(-offsets[bd]), nullptr);
          break;
        }
        case QNN_DATATYPE_UFIXED_POINT_8: {
          uint8_t* output = reinterpret_cast<uint8_t*>(quant_bytes.data());
          ParQuantizeLinearStd(&data[i], &output[i], block_size, scales[bd], static_cast<uint8_t>(-offsets[bd]), nullptr);
          break;
        }
        case QNN_DATATYPE_SFIXED_POINT_16: {
          int16_t* output = reinterpret_cast<int16_t*>(quant_bytes.data());
          ParQuantizeLinearStd(&data[i], &output[i], block_size, scales[bd], static_cast<int16_t>(-offsets[bd]), nullptr);
          break;
        }
        case QNN_DATATYPE_UFIXED_POINT_16: {
          uint16_t* output = reinterpret_cast<uint16_t*>(quant_bytes.data());
          ParQuantizeLinearStd(&data[i], &output[i], block_size, scales[bd], static_cast<uint16_t>(-offsets[bd]), nullptr);
          break;
        }
        case QNN_DATATYPE_SFIXED_POINT_32: {
          const double clip_min = static_cast<double>(std::numeric_limits<int32_t>::min());
          const double clip_max = static_cast<double>(std::numeric_limits<int32_t>::max());

          int32_t* output = reinterpret_cast<int32_t*>(quant_bytes.data());
          for (size_t e = 0; e < block_size; ++e) {
            const double scale = static_cast<double>(scales[bd]);
            const double offset = static_cast<double>(offsets[bd]);
            double float_val = std::nearbyint(static_cast<double>(data[i + e]) / scale) - offset;
            float_val = std::max(float_val, clip_min);
            float_val = std::min(float_val, clip_max);
            output[i + e] = static_cast<int32_t>(float_val);
          }
          break;
        }
        default:
          return ORT_MAKE_STATUS(ONNXRUNTIME, INVALID_ARGUMENT, "Unsupported quantization data type for QuantizeData");
      }
      i += block_size;
    }
  }
  assert(i == data.size());

  return Status::OK();
}

}  // namespace utils
}  // namespace qnn
}  // namespace onnxruntime
