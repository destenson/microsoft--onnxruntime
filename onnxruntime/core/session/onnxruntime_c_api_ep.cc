#include "core/session/onnxruntime_c_api_ep.h"
#include "ort_apis_ep.h"
#include "core/graph/graph_proto_serializer.h"
#include "core/graph/graph_viewer.h"
#include "core/graph/model.h"
#include "core/framework/error_code_helper.h"
#include "core/framework/murmurhash3.h"
#include "core/framework/session_options.h"
#include "core/framework/tensorprotoutils.h"
#include "core/session/ort_apis.h"

using namespace onnxruntime;

ORT_API(const char*, OrtGraphApis::OrtGraph_GetName, const OrtGraphViewer* graph) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return graph_viewer->Name().c_str();
}

ORT_API(bool, OrtGraphApis::OrtGraph_IsConstantInitializer, const OrtGraphViewer* graph, const char* name, bool check_outer_scope) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return graph_viewer->IsConstantInitializer(name, check_outer_scope);
}

ORT_API(size_t, OrtGraphApis::OrtGraph_GetNodesIndexInTopologicalOrder, const OrtGraphViewer* graph, int execution_order, _Out_ const size_t** nodes_index_in_topological_order) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  const std::vector<size_t>& nodes = graph_viewer->GetNodesInTopologicalOrder(static_cast<ExecutionOrder>(execution_order));
  *nodes_index_in_topological_order = nodes.data();
  return nodes.size();
}

ORT_API(bool, OrtGraphApis::OrtGraph_IsSubgraph, const OrtGraph* graph) {
  const ::onnxruntime::Graph* graph_ptr = reinterpret_cast<const ::onnxruntime::Graph*>(graph);
  return graph_ptr->IsSubgraph();
}

ORT_API(const OrtGraph*, OrtGraphApis::OrtGraph_GetParentGraph, const OrtGraph* graph) {
  const ::onnxruntime::Graph* graph_ptr = reinterpret_cast<const ::onnxruntime::Graph*>(graph);
  return reinterpret_cast<const OrtGraph*>(graph_ptr->ParentGraph());
}

ORT_API(const OrtNode*, OrtGraphApis::OrtGraph_GetParenNode, const OrtGraphViewer* graph) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return reinterpret_cast<const OrtNode*>(graph_viewer->ParentNode());
}

ORT_API(const void*, OrtGraphApis::OrtGraph_GetModelPath, const OrtGraphViewer* graph) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return reinterpret_cast<const void*>(&graph_viewer->ModelPath());
}

ORT_API(const OrtGraph*, OrtGraphApis::OrtGraph_GetOrtGraph, const OrtGraphViewer* graph_viewer) {
  const ::onnxruntime::GraphViewer* graph_viewer_ptr = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph_viewer);
  return reinterpret_cast<const OrtGraph*>(&graph_viewer_ptr->GetGraph());
}

ORT_API(size_t, OrtGraphApis::OrtGraph_GetInputsIncludingInitializers, const OrtGraphViewer* graph, _Outptr_ const char*** input_names) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  const auto& inputs = graph_viewer->GetInputsIncludingInitializers();
  size_t ret = inputs.size();
  *input_names = new const char*[ret];
  for (size_t i = 0; i < ret; i++) (*input_names)[i] = inputs[i]->Name().c_str();
  return ret;
}

ORT_API(const OrtNode*, OrtGraphApis::OrtGraph_GetOrtNode, const OrtGraphViewer* graph, size_t node_index) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return reinterpret_cast<const OrtNode*>(graph_viewer->GetNode(node_index));
}

ORT_API(size_t, OrtGraphApis::OrtGraph_GetNodesConsumingInput, const OrtGraphViewer* graph, const char* input_name, _Outptr_ const OrtNode*** consumers) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  std::vector<const ::onnxruntime::Node*> consumer_nodes = graph_viewer->GetConsumerNodes(input_name);
  size_t ret = consumer_nodes.size();
  *consumers = new const OrtNode* [ret];
  for (size_t i = 0; i < ret; i++) (*consumers)[i] = reinterpret_cast<const OrtNode*>(consumer_nodes[i]);

  return ret;
}

ORT_API(const OrtNode*, OrtGraphApis::OrtGraph_GetNodeProducingOutput, const OrtGraphViewer* graph, const char* output_name) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return reinterpret_cast<const OrtNode*>(graph_viewer->GetProducerNode(output_name));
}

ORT_API(int, OrtGraphApis::OrtGraph_NumberOfNodes, const OrtGraphViewer* graph) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return graph_viewer->NumberOfNodes();
}

ORT_API(int, OrtGraphApis::OrtGraph_MaxNodeIndex, const OrtGraphViewer* graph) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return graph_viewer->MaxNodeIndex();
}

ORT_API(size_t, OrtGraphApis::OrtGraph_GetOutputSize, const OrtGraphViewer* graph) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return graph_viewer->GetOutputs().size();
}

ORT_API(const char*, OrtGraphApis::OrtGraph_GetIthOutputName, const OrtGraphViewer* graph, size_t i) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return graph_viewer->GetOutputs()[i]->Name().c_str();
}

ORT_API(int32_t, OrtGraphApis::OrtGraph_GetIthOutputElemType, const OrtGraphViewer* graph, size_t i) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  return graph_viewer->GetOutputs()[i]->TypeAsProto()->tensor_type().elem_type();
}

ORT_API(bool, OrtGraphApis::OrtGraph_GetInitializerTensor, const OrtGraphViewer* graph, const char* initializer_name, _Outptr_ OrtTensorRef** out) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  const onnx::TensorProto* initializer = nullptr;
  if (!graph_viewer->GetInitializedTensor(initializer_name, initializer)) return false;
  *out = new OrtTensorRef();  // TODO(leca): release
  (*out)->shape_len = initializer->dims_size();
  (*out)->shape = new int64_t [initializer->dims_size()];
  for (size_t i = 0; i < (*out)->shape_len; i++) {
    ((*out)->shape)[i] = initializer->dims(i);
  }

  (*out)->data_type = static_cast<ONNXTensorElementDataType>(initializer->data_type());
  // see utils::ConvertRawDataInTensorProto()
  switch (initializer->data_type()) {
    case ONNX_NAMESPACE::TensorProto_DataType_FLOAT:
      (*out)->data_len = initializer->float_data_size();
      (*out)->data = reinterpret_cast<const char*>(initializer->float_data().data());
      break;
  }
  return true;
}

static ONNXTensorElementDataType GetDataTypeFromTypeProto(const onnx::TypeProto* type) {  // onnxruntime\core\optimizer\transpose_optimization\ort_optimizer_api_impl.cc
  if (!type || !utils::HasTensorType(*type) || !utils::HasElementType(*type)) return ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;

  return static_cast<ONNXTensorElementDataType>(type->tensor_type().elem_type());
}

ORT_API(bool, OrtGraphApis::OrtGraph_GetValueInfo, const OrtGraphViewer* graph, const char* name, _Outptr_ OrtValueInfoRef** out) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  const NodeArg* node_arg = graph_viewer->GetNodeArg(name);

  *out = new OrtValueInfoRef(); // TODO(leca): release
  const onnx::TypeProto* type = node_arg->TypeAsProto();
  (*out)->data_type = GetDataTypeFromTypeProto(type);
  const auto& dims = utils::TryGetShape(*type)->dim();
  (*out)->shape_len = dims.size();
  (*out)->shape = new int64_t [(*out)->shape_len];
  for (size_t i = 0; i < (*out)->shape_len; i++) ((*out)->shape)[i] = utils::HasDimValue(dims[i]) ? dims[i].dim_value() : -1;

  return true;
}

ORT_API(size_t, OrtGraphApis::OrtGraph_SerializeToArray, const OrtGraphViewer* graph, _Out_ void** data) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  Model model(graph_viewer->Name(), true, ModelMetaData(), PathString(),
#if defined(ORT_MINIMAL_BUILD)
    IOnnxRuntimeOpSchemaRegistryList(),
#else
    IOnnxRuntimeOpSchemaRegistryList({graph_viewer->GetSchemaRegistry()}),
#endif
    graph_viewer->DomainToVersionMap(), std::vector<onnx::FunctionProto>(), graph_viewer->GetGraph().GetLogger());
  onnx::ModelProto model_proto = model.ToProto();
  GraphViewerToProto(*graph_viewer, *model_proto.mutable_graph(), true, true, ExecutionOrder::PRIORITY_BASED);
  size_t ret = model_proto.ByteSizeLong();
  *data = malloc(ret);    // TODO(leca): release
  model_proto.SerializeToArray(*data, ret);
  return ret;
}

struct SubGraphContext2 {
  std::unordered_set<std::string> output_args;
  std::unordered_map<std::string, const NodeArg*> inputs_and_initializers;
  std::unordered_map<std::string, const NodeArg*> manually_added_graph_inputs;
};

static std::string GetUniqueGraphName(const Graph& graph) {
  HashValue model_hash = 0;
  uint32_t hash[4] = {0, 0, 0, 0};

  auto hash_str = [&hash](const std::string& str) {
    MurmurHash3::x86_128(str.data(), gsl::narrow_cast<int32_t>(str.size()), hash[0], &hash);
  };

  // Hash all nodes' name
  for (int i = 0; i < graph.MaxNodeIndex(); ++i) {
    auto node = graph.GetNode(i);
    if (node == nullptr) {
      continue;
    }
    hash_str(node->Name());
  }

  model_hash = hash[0] | (uint64_t(hash[1]) << 32);

  return graph.Name() + "_" + std::to_string(model_hash);
}

static bool IsLocalValue(const Graph& graph,
                                             const std::string& name,
                                             const std::unordered_map<std::string, std::unique_ptr<SubGraphContext2>>& subgraph_context_map) {
  std::string unique_graph_name = GetUniqueGraphName(graph);
  if (subgraph_context_map.find(unique_graph_name) == subgraph_context_map.end()) {
    return false;
  }
  SubGraphContext2* context = subgraph_context_map.at(unique_graph_name).get();
  return context->output_args.find(name) != context->output_args.cend() ||
         context->inputs_and_initializers.find(name) != context->inputs_and_initializers.cend();
}

static bool IsInputInitializerOrOutput(const Graph& graph,
                                                           const std::string& name,
                                                           bool check_ancestors,
                                                           const std::unordered_map<std::string, std::unique_ptr<SubGraphContext2>>& subgraph_context_map) {
  const Graph* parent_graph = nullptr;
  return IsLocalValue(graph, name, subgraph_context_map) ||
         (check_ancestors && (parent_graph = graph.ParentGraph()) != nullptr &&
          IsInputInitializerOrOutput(*parent_graph, name, check_ancestors, subgraph_context_map));
}

static bool IsOuterScopeValue(const Graph& graph,
                                                  const std::string& name,
                                                  const std::unordered_map<std::string, std::unique_ptr<SubGraphContext2>>& subgraph_context_map) {
  const Graph* parent_graph = nullptr;
  return (parent_graph = graph.ParentGraph()) != nullptr &&
         IsInputInitializerOrOutput(*parent_graph, name, true, subgraph_context_map);
}

static void BuildSubGraphContext(const Graph& graph, std::unordered_map<std::string, std::unique_ptr<SubGraphContext2>>& subgraph_context_map) {
  // Iterate all the nodes and recurse into inner most subgraph first
  for (int i = 0; i < graph.MaxNodeIndex(); ++i) {
    auto node = graph.GetNode(i);
    if (node == nullptr) {
      continue;
    }

    auto subgraph_map = node->GetAttributeNameToSubgraphMap();
    for (auto& entry : subgraph_map) {
      const Graph* subgraph = entry.second;
      BuildSubGraphContext(*subgraph, subgraph_context_map);
    }
  }

  std::string unique_graph_name = GetUniqueGraphName(graph);

  // Subgraph context has been built before, no need to do it again
  if (subgraph_context_map.find(unique_graph_name) != subgraph_context_map.end()) {
    return;
  }

  subgraph_context_map.emplace(unique_graph_name, std::make_unique<SubGraphContext2>());
  SubGraphContext2* context = subgraph_context_map.at(unique_graph_name).get();

  // Collect all nodes' outputs and nodes' name
  for (int i = 0; i < graph.MaxNodeIndex(); ++i) {
    auto node = graph.GetNode(i);
    if (node == nullptr) {
      continue;
    }

    for (const auto& output : node->OutputDefs()) {
      context->output_args.insert(output->Name());
    }
  }

  // Go thru all node's inputs
  for (int i = 0; i < graph.MaxNodeIndex(); ++i) {
    auto node = graph.GetNode(i);
    if (node == nullptr) {
      continue;
    }

    for (const auto& input : node->InputDefs()) {
      if (context->output_args.find(input->Name()) != context->output_args.end()) {
        continue;
      }
      // This input arg is not the output of another node so must come from either a graph input or an initializer.
      context->inputs_and_initializers[input->Name()] = input;
    }
  }
}

static void SetGraphOuterScopeValuesAndInputs(Graph& graph_build,
                                                                  const Graph& graph,
                                                                  std::unordered_map<std::string, std::unique_ptr<SubGraphContext2>>& subgraph_context_map) {
  // Iterate all the nodes and recurse into inner most subgraph first for both newly built graph and original graph
  for (int i = 0; i < graph_build.MaxNodeIndex(); ++i) {
    auto graph_build_node = graph_build.GetNode(i);
    if (graph_build_node == nullptr) {
      continue;
    }

    auto graph_build_map = graph_build_node->GetAttributeNameToMutableSubgraphMap();
    std::unordered_map<std::string, gsl::not_null<const Graph*>> subgraph_map;
    const Node* graph_node = nullptr;

    // Find corresponding original graph node's subgraphs
    for (int j = 0; j < graph.MaxNodeIndex(); ++j) {
      if (graph.GetNode(j) && graph.GetNode(j)->Name() == graph_build_node->Name()) {
        graph_node = graph.GetNode(j);
        subgraph_map = graph_node->GetAttributeNameToSubgraphMap();
        break;
      }
    }

    for (auto& entry : graph_build_map) {
      auto attr_name = entry.first;
      Graph* subgraph_build = entry.second;
      if (subgraph_map.find(attr_name) != subgraph_map.end()) {
        // recurse into subgraph
        const Graph* subgraph = subgraph_map.at(attr_name);
        SetGraphOuterScopeValuesAndInputs(*subgraph_build, *subgraph, subgraph_context_map);
      }
    }
  }

  // Start from the inner most subgraph first and check whether its outer scope values are existed in the
  // newly built graph. If not, we need to add those outer scope values as explicit inputs to the top-level
  // of newly built graph.
  if (graph_build.ParentNode()) {
    auto top_level_graph = &graph_build;
    while (top_level_graph->MutableParentGraph()) {
      top_level_graph = top_level_graph->MutableParentGraph();
    }
    std::string unique_graph_name = GetUniqueGraphName(*top_level_graph);
    if (subgraph_context_map.find(unique_graph_name) == subgraph_context_map.end()) {
      return;
    }

    SubGraphContext2* context = subgraph_context_map.at(unique_graph_name).get();

    // Iterate all the implicit inputs to set outer scope value for the newly built subgraph
    for (const auto& input : graph.ParentNode()->ImplicitInputDefs()) {
//      LOGS_DEFAULT(VERBOSE) << "[TensorRT EP] \t" << input->Name();

      // The node arg in parent node's implicit inputs could be used for parent node's other subgraph, for example
      // "If" op has two subgraphs. So we need to make sure that the node arg is used in current subgraph only.
      // (GetNodeArg searches for specific node arg in all node args in the graph)
      if (graph_build.GetNodeArg(input->Name())) {
        graph_build.AddOuterScopeNodeArg(input->Name());
//        LOGS_DEFAULT(VERBOSE) << "[TensorRT EP] \t" << input->Name() << " is used in this subgraph";

        if (context &&
            (context->manually_added_graph_inputs.find(input->Name()) != context->manually_added_graph_inputs.end())) {
//          LOGS_DEFAULT(VERBOSE) << "[TensorRT EP] \t" << input->Name() << " is already been added as an explicit input to graph";
          continue;
        }

        // Handle the case where this outer scope value is not existed in any outer scope levels of the
        // newly built graph (the newly built graph is the subgraph of the original graph). Need to add
        // the outer scope value as an explicit input to the top-level of newly built graph.
        if (!IsOuterScopeValue(graph_build, input->Name(), subgraph_context_map)) {
          const auto& name = input->Name();
          auto graph_inputs_including_initializers = top_level_graph->GetInputsIncludingInitializers();
          auto added_graph_input = std::find_if(graph_inputs_including_initializers.begin(),
                                                graph_inputs_including_initializers.end(),
                                                [&name](const NodeArg* entry) { return entry->Name() == name; });

          if (added_graph_input == graph_inputs_including_initializers.end()) {
            if (context) {
              auto type_proto = std::make_unique<ONNX_NAMESPACE::TypeProto>();
              type_proto->CopyFrom(*(input->TypeAsProto()));
              auto& n_input = top_level_graph->GetOrCreateNodeArg(name, type_proto.get());
              context->manually_added_graph_inputs[n_input.Name()] = &n_input;
//              LOGS_DEFAULT(VERBOSE) << "[TensorRT EP] \t" << n_input.Name() << " is added as an explicit input into the newly built graph";
            }
          }
        }
      }
    }
  }
}

static void SetAllGraphInputs(Graph& graph, std::unordered_map<std::string, std::unique_ptr<SubGraphContext2>>& subgraph_context_map) {
  // If ORT TRT doesn't manully set graph input in TensorrtExecutionProvider::SetGraphOuterScopeValuesAndInputs(),
  // Graph::Resolve() will help set graph inputs in Graph::SetGraphInputsOutputs(), so no need to set graph inputs here.
  std::string unique_graph_name = GetUniqueGraphName(graph);
  if (subgraph_context_map.find(unique_graph_name) == subgraph_context_map.end() ||
      subgraph_context_map[unique_graph_name].get()->manually_added_graph_inputs.size() == 0) {
    return;
  }

  SubGraphContext2* context = subgraph_context_map[unique_graph_name].get();
  std::vector<const NodeArg*> graph_inputs_including_initializers;
  std::unordered_set<std::string> graph_inputs_including_initializers_set;

  for (const auto& entry : context->inputs_and_initializers) {
    graph_inputs_including_initializers.push_back(entry.second);
    graph_inputs_including_initializers_set.insert(entry.first);
  }

  for (const auto& entry : context->manually_added_graph_inputs) {
    if (graph_inputs_including_initializers_set.find(entry.first) == graph_inputs_including_initializers_set.end()) {
      graph_inputs_including_initializers.push_back(entry.second);
      graph_inputs_including_initializers_set.insert(entry.first);
    }
  }

  for (const auto& node_arg : graph.GetInputsIncludingInitializers()) {
    if (graph_inputs_including_initializers_set.find(node_arg->Name()) == graph_inputs_including_initializers_set.end()) {
      graph_inputs_including_initializers.push_back(node_arg);
      graph_inputs_including_initializers_set.insert(node_arg->Name());
    }
  }

  graph.SetInputs(graph_inputs_including_initializers);
}

ORT_API_STATUS_IMPL(OrtGraphApis::OrtGraph_GetSubGraph, const OrtGraphViewer* graph, const int node_num, const size_t* node_indices, _Outptr_ const OrtGraphViewer** subgraph) {
  const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
  // Get parent graph output names
  std::unordered_set<std::string> graph_output_names;
  for (const auto* output_arg : graph_viewer->GetOutputs()) {
    graph_output_names.insert(output_arg->Name());
  }
  // TODO(leca): cannot use unique_ptr here, otherwise when this function exits, sub_graph_viewer->graph_->graph_proto_, which is from model_build->model_proto_, will be nullptr.
  // Pay special attention when Graph object is releasing. We need to release model_build seperately then.
  Model* model_build = new Model (graph_viewer->Name(), true, ModelMetaData(), PathString(),
#if !defined(ORT_MINIMAL_BUILD)
                                   IOnnxRuntimeOpSchemaRegistryList({graph_viewer->GetSchemaRegistry()}), graph_viewer->DomainToVersionMap(),
#else
                                   IOnnxRuntimeOpSchemaRegistryList(), graph_viewer->DomainToVersionMap(),
#endif  // ORT_MINIMAL_BUILD
                                   std::vector<ONNX_NAMESPACE::FunctionProto>(), graph_viewer->GetGraph().GetLogger());

  auto& graph_build = model_build->MainGraph();
  bool has_control_flow_op = false;

  std::vector<std::string> subgraph_output_names;
  const std::vector<NodeIndex>& node_index = graph_viewer->GetNodesInTopologicalOrder(ExecutionOrder::PRIORITY_BASED);
  for(int i = 0; i < node_num; i++) {
    const auto& node = graph_viewer->GetNode(node_index[node_indices[i]]);
    std::vector<onnxruntime::NodeArg*> inputs, outputs;
    for (auto input : node->InputDefs()) {
      auto& n_input = graph_build.GetOrCreateNodeArg(input->Name(), input->TypeAsProto());
      inputs.push_back(&n_input);
      const ONNX_NAMESPACE::TensorProto* initializer = nullptr;
      if (graph_viewer->GetInitializedTensor(input->Name(), initializer)) {
        const ONNX_NAMESPACE::TensorProto* subgraph_initializer = nullptr;
        if (!graph_build.GetInitializedTensor(input->Name(), subgraph_initializer)) {
          graph_build.AddInitializedTensor(*(initializer));
        }
      }
    }
    for (auto input : node->ImplicitInputDefs()) {
      const ONNX_NAMESPACE::TensorProto* initializer = nullptr;
      if (graph_viewer->GetInitializedTensor(input->Name(), initializer)) {
        const ONNX_NAMESPACE::TensorProto* subgraph_initializer = nullptr;
        if (!graph_build.GetInitializedTensor(input->Name(), subgraph_initializer)) {
          graph_build.AddInitializedTensor(*(initializer));
        }
      }
    }
    for (auto output : node->OutputDefs()) {
      auto& n_output = graph_build.GetOrCreateNodeArg(output->Name(), output->TypeAsProto());
      outputs.push_back(&n_output);
      const auto name = output->Name();
      if (graph_output_names.find(name) != graph_output_names.end()) {
        subgraph_output_names.push_back(name);
      }
    }

    std::unordered_set<std::string> control_flow_op_set = {"If", "Loop", "Scan"};
    if (control_flow_op_set.find(node->OpType()) != control_flow_op_set.end()) {
      has_control_flow_op = true;
    }

    // If the node has subgraph, it's possible that the ORT graph of that subgraph and the GraphProto in the node attributes are not in sync because of graph optimization.
    // Therefore, we need to force GraphProto attributes to be updated in order to get the valid GraphProto.
    if (node->GetAttributes().size() > 0) {
      auto node_proto = std::make_unique<ONNX_NAMESPACE::NodeProto>();
      // we need to update any GraphProto attributes for subgraphs so that any changes made by things
      // such as the optimizers are captured. otherwise we can end up saving an invalid graph.
      node->ToProto(*node_proto, /* update_subgraphs */ true);
      const int num_attributes = node_proto->attribute_size();
      NodeAttributes node_attributes;
      node_attributes.reserve(num_attributes);

      for (int i = 0; i < num_attributes; ++i) {
        auto& attr = node_proto->attribute(i);
        node_attributes.emplace(attr.name(), attr);
      }

      // The GraphProto attributes are the updated ones.
      graph_build.AddNode(node->Name(), node->OpType(), node->Description(), inputs, outputs, &node_attributes, node->Domain());
    } else {
      // The GraphProto attributes are the original ones.
      graph_build.AddNode(node->Name(), node->OpType(), node->Description(), inputs, outputs, &node->GetAttributes(), node->Domain());
    }
  }

  // TODO:yang
  // Only if the newly built graph has control flow op as well as it has parent node,
  // it needs to handle outer scope values before calling graph.Resolve().
  // TODO(leca): Is local variable enough? Do we need to make it EP class variable?
  std::unordered_map<std::string, std::unique_ptr<SubGraphContext2>> subgraph_context_map;
  if (has_control_flow_op && graph_viewer->ParentNode()) {
  //   LOGS_DEFAULT(VERBOSE) << "[TensorRT EP] Handle outer scope values for the subgraph " << graph_build.Name();
     BuildSubGraphContext(graph_build, subgraph_context_map);
     SetGraphOuterScopeValuesAndInputs(graph_build, graph_viewer->GetGraph(), subgraph_context_map);
     SetAllGraphInputs(graph_build, subgraph_context_map);
  }

  common::Status status = graph_build.Resolve();
  if (status != Status::OK()) return onnxruntime::ToOrtStatus(status);

  // Add parent graph output to the subgraph
  int i = 0;
  std::vector<const NodeArg*> subgraph_outputs;
  subgraph_outputs.resize(subgraph_output_names.size());
  for (auto& name : subgraph_output_names) {
    auto output_arg = graph_viewer->GetNodeArg(name);
    auto& subgraph_output_arg = graph_build.GetOrCreateNodeArg(output_arg->Name(), output_arg->TypeAsProto());
    subgraph_outputs[i] = &subgraph_output_arg;
    ++i;
  }
  auto& graph_build_outputs = graph_build.GetOutputs();
  subgraph_outputs.insert(subgraph_outputs.begin(), graph_build_outputs.begin(), graph_build_outputs.end());
  graph_build.SetOutputs(graph_build_outputs);
  status = graph_build.Resolve();
  if (status != Status::OK()) return onnxruntime::ToOrtStatus(status);

  auto sub_graph_viewer = std::make_unique<GraphViewer>(graph_build);
  *subgraph = reinterpret_cast<const OrtGraphViewer*>(sub_graph_viewer.release());
  return nullptr;
}

ORT_API_STATUS_IMPL(OrtGraphApis::OrtGraph_ReleaseGraph, const OrtGraphViewer* graph) {
  if (graph) {
    const ::onnxruntime::GraphViewer* graph_viewer = reinterpret_cast<const ::onnxruntime::GraphViewer*>(graph);
    delete graph_viewer;
  }
  return nullptr;
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetName, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->Name().c_str();
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetDescription, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->Description().c_str();
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetDomain, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->Domain().c_str();
}

ORT_API(int, OrtGraphApis::OrtNode_SinceVersion, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->SinceVersion();
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetExecutionProviderType, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetExecutionProviderType().c_str();
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetOpType, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->OpType().c_str();
}

ORT_API(size_t, OrtGraphApis::OrtNode_GetImplicitInputSize, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->ImplicitInputDefs().size();
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetIthImplicitInputName, const OrtNode* node, size_t i) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  assert(i < n->ImplicitInputDefs().size());
  return n->ImplicitInputDefs()[i]->Name().c_str();
}

ORT_API(size_t, OrtGraphApis::OrtNode_GetInputSize, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->InputDefs().size();
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetIthInputName, const OrtNode* node, size_t i) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  assert(i < n->InputDefs().size());
  return n->InputDefs()[i]->Name().c_str();
}

ORT_API(size_t, OrtGraphApis::OrtNode_GetOutputSize, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->OutputDefs().size();
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetIthOutputName, const OrtNode* node, size_t i) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  assert(i < n->OutputDefs().size());
  if (n->OutputDefs()[i]->Exists()) return n->OutputDefs()[i]->Name().c_str();
  return nullptr;
}

ORT_API(size_t, OrtGraphApis::OrtNode_GetIndex, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->Index();
}

ORT_API(size_t, OrtGraphApis::OrtNode_GetAttributeNames, const OrtNode* node, _Out_ const char*** names) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  size_t ret = n->GetAttributes().size();
  *names = new const char* [ret];
  int i = 0;
  for (const auto& [k, v] : n->GetAttributes()) {
    (*names)[i++] = k.c_str();
  }
  return ret;
}

ORT_API(size_t, OrtGraphApis::OrtNode_GetAttributeSize, const OrtNode* node) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().size();
}

ORT_API(int, OrtGraphApis::OrtNode_GetAttributeType, const OrtNode* node, const char* attribute) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return static_cast<int>(n->GetAttributes().at(attribute).type());
}

ORT_API(size_t, OrtGraphApis::OrtNode_GetAttributeKeyCount, const OrtNode* node, const char* key) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().count(key);
}

ORT_API(int, OrtGraphApis::OrtNode_GetAttributeIntSize, const OrtNode* node, const char* key) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).ints_size();
}

ORT_API(int, OrtGraphApis::OrtNode_GetAttributeFloatSize, const OrtNode* node, const char* key) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).floats_size();
}

ORT_API(int, OrtGraphApis::OrtNode_GetAttributeStringSize, const OrtNode* node, const char* key) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).strings_size();
}

ORT_API(int64_t, OrtGraphApis::OrtNode_GetAttributeIthInt, const OrtNode* node, const char* key, int i) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).ints(i);
}

ORT_API(float, OrtGraphApis::OrtNode_GetAttributeIthFloat, const OrtNode* node, const char* key, int i) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).floats(i);
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetAttributeIthStr, const OrtNode* node, const char* key, int i) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).strings(i).c_str();
}

ORT_API(const char*, OrtGraphApis::OrtNode_GetAttributeStr, const OrtNode* node, const char* key) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).s().c_str();
}

ORT_API(int64_t, OrtGraphApis::OrtNode_GetAttributeInt, const OrtNode* node, const char* key) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).i();
}

ORT_API(float, OrtGraphApis::OrtNode_GetAttributeFloat, const OrtNode* node, const char* key) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  return n->GetAttributes().at(key).f();
}

ORT_API(size_t, OrtGraphApis::OrtNode_GetSubgraphs, const OrtNode* node, _Outptr_ const OrtGraphViewer*** subgraphs) {
  const ::onnxruntime::Node* n = reinterpret_cast<const ::onnxruntime::Node*>(node);
  std::vector<gsl::not_null<const Graph*>> subg = n->GetSubgraphs();
  size_t ret = subg.size();
  *subgraphs = new const OrtGraphViewer* [ret];
  for (size_t i = 0; i < ret; i++) {
    const ::onnxruntime::GraphViewer* graph_viewer = new const ::onnxruntime::GraphViewer(*subg[i]);
    (*subgraphs)[i] = reinterpret_cast<const OrtGraphViewer*>(graph_viewer);
  }
  return ret;
}

ORT_API_STATUS_IMPL(OrtGraphApis::OrtFreeMem, void* p) {
  if (p) {
    free(p);
  }
  return nullptr;
}

static constexpr OrtGraphApi ort_graph_api = {
    &OrtGraphApis::OrtGraph_GetName,
    &OrtGraphApis::OrtGraph_IsConstantInitializer,
    &OrtGraphApis::OrtGraph_GetNodesIndexInTopologicalOrder,
    &OrtGraphApis::OrtGraph_IsSubgraph,
    &OrtGraphApis::OrtGraph_GetParentGraph,
    &OrtGraphApis::OrtGraph_GetParenNode,
    &OrtGraphApis::OrtGraph_GetModelPath,
    &OrtGraphApis::OrtGraph_GetOrtGraph,
    &OrtGraphApis::OrtGraph_GetInputsIncludingInitializers,
    &OrtGraphApis::OrtGraph_GetOrtNode,
    &OrtGraphApis::OrtGraph_GetNodesConsumingInput,
    &OrtGraphApis::OrtGraph_GetNodeProducingOutput,
    &OrtGraphApis::OrtGraph_NumberOfNodes,
    &OrtGraphApis::OrtGraph_MaxNodeIndex,
    &OrtGraphApis::OrtGraph_GetOutputSize,
    &OrtGraphApis::OrtGraph_GetIthOutputName,
    &OrtGraphApis::OrtGraph_GetIthOutputElemType,
    &OrtGraphApis::OrtGraph_GetInitializerTensor,
    &OrtGraphApis::OrtGraph_GetValueInfo,
    &OrtGraphApis::OrtGraph_SerializeToArray,
    &OrtGraphApis::OrtGraph_GetSubGraph,
    &OrtGraphApis::OrtGraph_ReleaseGraph,
    &OrtGraphApis::OrtNode_GetName,
    &OrtGraphApis::OrtNode_GetDescription,
    &OrtGraphApis::OrtNode_GetDomain,
    &OrtGraphApis::OrtNode_SinceVersion,
    &OrtGraphApis::OrtNode_GetExecutionProviderType,
    &OrtGraphApis::OrtNode_GetOpType,
    &OrtGraphApis::OrtNode_GetImplicitInputSize,
    &OrtGraphApis::OrtNode_GetIthImplicitInputName,
    &OrtGraphApis::OrtNode_GetInputSize,
    &OrtGraphApis::OrtNode_GetIthInputName,
    &OrtGraphApis::OrtNode_GetOutputSize,
    &OrtGraphApis::OrtNode_GetIthOutputName,
    &OrtGraphApis::OrtNode_GetIndex,
    &OrtGraphApis::OrtNode_GetAttributeNames,
    &OrtGraphApis::OrtNode_GetAttributeSize,
    &OrtGraphApis::OrtNode_GetAttributeType,
    &OrtGraphApis::OrtNode_GetAttributeKeyCount,
    &OrtGraphApis::OrtNode_GetAttributeIntSize,
    &OrtGraphApis::OrtNode_GetAttributeFloatSize,
    &OrtGraphApis::OrtNode_GetAttributeStringSize,
    &OrtGraphApis::OrtNode_GetAttributeIthInt,
    &OrtGraphApis::OrtNode_GetAttributeIthFloat,
    &OrtGraphApis::OrtNode_GetAttributeIthStr,
    &OrtGraphApis::OrtNode_GetAttributeStr,
    &OrtGraphApis::OrtNode_GetAttributeInt,
    &OrtGraphApis::OrtNode_GetAttributeFloat,
    &OrtGraphApis::OrtNode_GetSubgraphs,
    &OrtGraphApis::OrtFreeMem,
};

ORT_API(const OrtGraphApi*, OrtGraphApis::GetGraphApi, uint32_t) {
  // No constraints on the API version yet.
  return &ort_graph_api;
}
