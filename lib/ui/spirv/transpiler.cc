// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "lib/ui/spirv/transpiler.h"

#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "third_party/spirv_headers/include/spirv/unified1/GLSL.std.450.h"
#include "third_party/spirv_headers/include/spirv/unified1/spirv.hpp"
#include "third_party/spirv_tools/include/spirv-tools/libspirv.h"

namespace flutter {
namespace spirv {

class TranspilerImpl : public Transpiler {
 public:
  TranspilerImpl();
  virtual ~TranspilerImpl();

  virtual Result Transpile(const char* data, size_t length) override;
  virtual std::string GetSkSL() override;

  void set_last_op(uint32_t op);
  void set_last_msg(std::string msg);

  spv_result_t HandleCapability(const spv_parsed_instruction_t* inst);
  spv_result_t HandleExtInstImport(const spv_parsed_instruction_t* inst);
  spv_result_t HandleMemoryModel(const spv_parsed_instruction_t* inst);
  spv_result_t HandleDecorate(const spv_parsed_instruction_t* inst);
  spv_result_t HandleTypeFloat(const spv_parsed_instruction_t* inst);
  spv_result_t HandleTypeVector(const spv_parsed_instruction_t* inst);
  spv_result_t HandleTypePointer(const spv_parsed_instruction_t* inst);
  spv_result_t HandleTypeFunction(const spv_parsed_instruction_t* inst);
  spv_result_t HandleConstant(const spv_parsed_instruction_t* inst);
  spv_result_t HandleConstantComposite(const spv_parsed_instruction_t* inst);
  spv_result_t HandleVariable(const spv_parsed_instruction_t* inst);
  spv_result_t HandleFunction(const spv_parsed_instruction_t* inst);
  spv_result_t HandleFunctionParameter(const spv_parsed_instruction_t* inst);
  spv_result_t HandleFunctionCall(const spv_parsed_instruction_t* inst);
  spv_result_t HandleLabel(const spv_parsed_instruction_t* inst);
  spv_result_t HandleReturnValue(const spv_parsed_instruction_t* inst);
  spv_result_t HandleCompositeConstruct(const spv_parsed_instruction_t* inst);
  spv_result_t HandleCompositeExtract(const spv_parsed_instruction_t* inst);
  spv_result_t HandleLoad(const spv_parsed_instruction_t* inst);
  spv_result_t HandleFNegate(const spv_parsed_instruction_t* inst);
  spv_result_t HandleOperator(const spv_parsed_instruction_t* inst, char op);
  spv_result_t HandleBuiltin(const spv_parsed_instruction_t* inst,
                             std::string name);
  spv_result_t HandleExtInst(const spv_parsed_instruction_t* inst);
  spv_result_t HandleFunctionEnd(const spv_parsed_instruction_t* inst);

 private:
  std::string ResolveName(uint32_t id);
  std::string ResolveType(uint32_t id);
  std::string ResolveGLSLName(uint32_t id);

  const spv_context spv_context_;
  spv_diagnostic spv_diagnostic_;

  const uint32_t* words_;
  size_t word_count_;

  std::string last_error_msg_ = "";

  // Result-IDs of important instructions.
  uint32_t main_function_type_ = 0;
  uint32_t float_type_ = 0;
  uint32_t vec2_type_ = 0;
  uint32_t vec3_type_ = 0;
  uint32_t vec4_type_ = 0;
  uint32_t float_uniform_type_ = 0;
  uint32_t vec2_uniform_type_ = 0;
  uint32_t vec3_uniform_type_ = 0;
  uint32_t vec4_uniform_type_ = 0;
  uint32_t main_function_ = 0;
  uint32_t frag_position_param_ = 0;
  uint32_t return_ = 0;
  uint32_t last_op_ = 0;

  std::unordered_map<uint32_t, std::string> imported_functions_;

  std::stringstream sksl_;
};

namespace {

uint32_t get_operand(const spv_parsed_instruction_t* parsed_instruction,
                     int operand_index) {
  return parsed_instruction
      ->words[parsed_instruction->operands[operand_index].offset];
}

const char* get_literal(const spv_parsed_instruction_t* parsed_instruction,
                        int operand_index) {
  return reinterpret_cast<const char*>(
      &parsed_instruction
           ->words[parsed_instruction->operands[operand_index].offset]);
}

spv_result_t parse_header(void* user_data,
                          spv_endianness_t endian,
                          uint32_t magic,
                          uint32_t version,
                          uint32_t generator,
                          uint32_t id_bound,
                          uint32_t reserverd) {
  return SPV_SUCCESS;
}

spv_result_t parse_instruction(
    void* user_data,
    const spv_parsed_instruction_t* parsed_instruction) {
  auto* interpreter = static_cast<TranspilerImpl*>(user_data);
  spv_result_t result = SPV_UNSUPPORTED;

  switch (parsed_instruction->opcode) {
    case spv::OpCapability:
      result = interpreter->HandleCapability(parsed_instruction);
      break;
    case spv::OpExtInstImport:
      result = interpreter->HandleExtInstImport(parsed_instruction);
      break;
    case spv::OpMemoryModel:
      result = interpreter->HandleMemoryModel(parsed_instruction);
      break;
    case spv::OpDecorate:
      result = interpreter->HandleDecorate(parsed_instruction);
      break;
    case spv::OpTypeFloat:
      result = interpreter->HandleTypeFloat(parsed_instruction);
      break;
    case spv::OpTypeVector:
      result = interpreter->HandleTypeVector(parsed_instruction);
      break;
    case spv::OpTypePointer:
      result = interpreter->HandleTypePointer(parsed_instruction);
      break;
    case spv::OpTypeFunction:
      result = interpreter->HandleTypeFunction(parsed_instruction);
      break;
    case spv::OpConstant:
      result = interpreter->HandleConstant(parsed_instruction);
      break;
    case spv::OpConstantComposite:
      result = interpreter->HandleConstantComposite(parsed_instruction);
      break;
    case spv::OpVariable:
      result = interpreter->HandleVariable(parsed_instruction);
      break;
    case spv::OpFunction:
      result = interpreter->HandleFunction(parsed_instruction);
      break;
    case spv::OpFunctionParameter:
      result = interpreter->HandleFunctionParameter(parsed_instruction);
      break;
    case spv::OpFunctionCall:
      result = interpreter->HandleFunctionCall(parsed_instruction);
      break;
    case spv::OpLabel:
      result = interpreter->HandleLabel(parsed_instruction);
      break;
    case spv::OpReturnValue:
      result = interpreter->HandleReturnValue(parsed_instruction);
      break;
    case spv::OpCompositeConstruct:
      result = interpreter->HandleCompositeConstruct(parsed_instruction);
      break;
    case spv::OpCompositeExtract:
      result = interpreter->HandleCompositeExtract(parsed_instruction);
      break;
    case spv::OpLoad:
      result = interpreter->HandleLoad(parsed_instruction);
      break;
    case spv::OpFNegate:
      result = interpreter->HandleFNegate(parsed_instruction);
      break;
    case spv::OpFAdd:
      result = interpreter->HandleOperator(parsed_instruction, '+');
      break;
    case spv::OpFSub:
      result = interpreter->HandleOperator(parsed_instruction, '-');
      break;
    case spv::OpFMul:
    case spv::OpVectorTimesScalar:
    case spv::OpVectorTimesMatrix:
    case spv::OpMatrixTimesVector:
    case spv::OpMatrixTimesMatrix:
      result = interpreter->HandleOperator(parsed_instruction, '*');
      break;
    case spv::OpFDiv:
      result = interpreter->HandleOperator(parsed_instruction, '/');
      break;
    case spv::OpFMod:
      result = interpreter->HandleBuiltin(parsed_instruction, "mod");
      break;
    case spv::OpDot:
      result = interpreter->HandleBuiltin(parsed_instruction, "dot");
      break;
    case spv::OpExtInst:
      result = interpreter->HandleExtInst(parsed_instruction);
      break;
    case spv::OpFunctionEnd:
      result = interpreter->HandleFunctionEnd(parsed_instruction);
      break;
    default:
      interpreter->set_last_msg("Unsupported OP: " +
                                std::to_string(parsed_instruction->opcode));
      return SPV_UNSUPPORTED;
  }

  interpreter->set_last_op(parsed_instruction->opcode);
  return result;
}

}  // namespace

std::unique_ptr<Transpiler> Transpiler::create() {
  return std::make_unique<TranspilerImpl>();
}

TranspilerImpl::TranspilerImpl()
    : spv_context_(spvContextCreate(SPV_ENV_UNIVERSAL_1_5)) {}

TranspilerImpl::~TranspilerImpl() {
  if (spv_context_ != NULL) {
    spvContextDestroy(spv_context_);
  }
}

Result TranspilerImpl::Transpile(const char* data, size_t length) {
  if (spv_context_ == NULL) {
    return {.status = kFailedToInitialize,
            .message = "Failed to create SPIR-V Tools context."};
  }

  if (length % 4 != 0) {
    return {
        .status = kInvalidData,
        .message = "Provided data was not an integer number of 32-bit words"};
  }

  words_ = reinterpret_cast<const uint32_t*>(data);
  word_count_ = length / 4;

  spv_result_t result = spvBinaryParse(spv_context_,
                                       this,  // user_data
                                       words_, word_count_, &parse_header,
                                       &parse_instruction, &spv_diagnostic_);

  if (result != SPV_SUCCESS) {
    sksl_.str("");
    return {.status = kFailure,
            .message = last_error_msg_.empty()
                           ? "spv error code " + std::to_string(result) +
                                 " on op " + std::to_string(last_op_)
                           : last_error_msg_};
  }

  return {.status = kSuccess};
}

std::string TranspilerImpl::GetSkSL() {
  return sksl_.str();
}

void TranspilerImpl::set_last_op(uint32_t op) {
  last_op_ = op;
}

void TranspilerImpl::set_last_msg(std::string msg) {
  last_error_msg_ = msg;
}

std::string TranspilerImpl::ResolveName(uint32_t id) {
  return "i" + std::to_string(id);
}

std::string TranspilerImpl::ResolveType(uint32_t id) {
  if (id == float_type_ || id == float_uniform_type_) {
    return "float";
  } else if (id == vec2_type_ || id == vec2_uniform_type_) {
    return "float2";
  } else if (id == vec3_type_ || id == vec3_uniform_type_) {
    return "float3";
  } else if (id == vec4_type_ || id == vec4_uniform_type_) {
    return "float4";
  }
  return "";
}

spv_result_t TranspilerImpl::HandleCapability(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kCapabilityIndex = 0;
  uint32_t capability = get_operand(inst, kCapabilityIndex);
  switch (capability) {
    case spv::CapabilityMatrix:
    case spv::CapabilityShader:
    case spv::CapabilityLinkage:
      return SPV_SUCCESS;
    default:
      last_error_msg_ = "OpCapability: Capability " +
                        std::to_string(capability) + " is unsupported.";
      return SPV_UNSUPPORTED;
  }
}

spv_result_t TranspilerImpl::HandleExtInstImport(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kNameIndex = 0;
  static constexpr char kGLSLImportName[] = "GLSL.std.450";

  const char* name = reinterpret_cast<const char*>(
      &inst->words[inst->operands[kNameIndex].offset]);
  if (!strcmp(kGLSLImportName, name)) {
    last_error_msg_ = "OpExtInstImport: '" + std::string(kGLSLImportName) +
                      "' is not supported.";
    return SPV_UNSUPPORTED;
  }

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleMemoryModel(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kAddressingModelIndex = 0;
  static constexpr int kMemoryModelIndex = 1;

  uint32_t addressing_model = get_operand(inst, kAddressingModelIndex);
  if (addressing_model != spv::AddressingModelLogical) {
    last_error_msg_ =
        "OpMemoryModel: Only `Logical` addressing model is supported.";
    return SPV_UNSUPPORTED;
  }

  uint32_t memory_model = get_operand(inst, kMemoryModelIndex);
  if (memory_model != spv::MemoryModelGLSL450) {
    last_error_msg_ =
        "OpMemoryModel: Only memory model `GLSL450` is supported.";
    return SPV_UNSUPPORTED;
  }
  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleDecorate(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kTargetIndex = 0;
  static constexpr int kDecorationIndex = 1;
  static constexpr int kLinkageName = 2;
  static constexpr int kLinkageType = 3;
  static constexpr char kMainExportName[] = "main";

  if (get_operand(inst, kDecorationIndex) != spv::DecorationLinkageAttributes) {
    last_error_msg_ = "OpDecorate: Only LinkageAttributes are supported.";
    return SPV_UNSUPPORTED;
  }

  auto linkage_type =
      static_cast<spv::LinkageType>(get_operand(inst, kLinkageType));

  const char* name_cstr = get_literal(inst, kLinkageName);
  std::string name(name_cstr);
  if (strcmp(name_cstr, kMainExportName) == 0) {
    if (linkage_type != spv::LinkageTypeExport) {
      last_error_msg_ = "OpDecorate: Main must be an exported function.";
      return SPV_UNSUPPORTED;
    }

    main_function_ = get_operand(inst, kTargetIndex);
    return SPV_SUCCESS;
  }

  if (linkage_type != spv::LinkageTypeImport) {
    // Ignore all non-main exports.
    return SPV_SUCCESS;
  }

  imported_functions_[get_operand(inst, kTargetIndex)] = name;

  sksl_ << "in shader " << name << ";\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleTypeFloat(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kWidthIndex = 1;
  static constexpr uint32_t kRequiredFloatWidth = 32;
  uint32_t width = get_operand(inst, kWidthIndex);
  if (width != kRequiredFloatWidth) {
    last_error_msg_ =
        "OpTypeFloat: Only 32-bit width is supported. "
        "Got width " +
        std::to_string(width);
    return SPV_UNSUPPORTED;
  }

  if (float_type_ != 0) {
    last_error_msg_ = "OpTypeFloat: Only one OpTypeFloat should be specified.";
    return SPV_UNSUPPORTED;
  }

  float_type_ = inst->result_id;
  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleTypeVector(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kComponentTypeIndex = 1;
  static constexpr int kComponentCountIndex = 2;
  uint32_t type = get_operand(inst, kComponentTypeIndex);
  if (type == 0 || type != float_type_) {
    last_error_msg_ =
        "OpTypeVector: OpTypeFloat was not declared, "
        "or didn't match the given component type.";
    return SPV_ERROR_INVALID_VALUE;
  }

  uint32_t count = get_operand(inst, kComponentCountIndex);

  switch (count) {
    case 2:
      vec2_type_ = inst->result_id;
      break;
    case 3:
      vec3_type_ = inst->result_id;
      break;
    case 4:
      vec4_type_ = inst->result_id;
      break;
    default:
      last_error_msg_ = "OpTypeVector: Component count must be 2, 3, or 4.";
      return SPV_UNSUPPORTED;
  }

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleTypePointer(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kStorageClassIndex = 1;
  static constexpr int kTypeIndex = 2;
  uint32_t type = get_operand(inst, kTypeIndex);
  uint32_t storage_class = get_operand(inst, kStorageClassIndex);

  if (storage_class != spv::StorageClassUniformConstant) {
    last_error_msg_ =
        "OpTypePointer: Only storage class 'UniformConstant' is supported.";
    return SPV_UNSUPPORTED;
  }

  if (type == float_type_) {
    float_uniform_type_ = inst->result_id;
  } else if (type == vec2_type_) {
    vec2_uniform_type_ = inst->result_id;
  } else if (type == vec3_type_) {
    vec3_uniform_type_ = inst->result_id;
  } else if (type == vec4_type_) {
    vec4_uniform_type_ = inst->result_id;
  } else {
    last_error_msg_ = "OpTypePointer: Must be a supported SSIR type.";
    return SPV_UNSUPPORTED;
  }

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleTypeFunction(
    const spv_parsed_instruction_t* inst) {
  if (main_function_type_ != 0) {
    last_error_msg_ =
        "OpTypeFunction: Only a single function type is supported.";
    return SPV_UNSUPPORTED;
  }

  if (inst->num_operands > 3) {
    last_error_msg_ = "OpTypeFunction: Only one parameter is supported.";
    return SPV_UNSUPPORTED;
  }

  static constexpr int kParameterIndex = 2;
  uint32_t param_type_id = get_operand(inst, kParameterIndex);
  if (param_type_id == 0 || param_type_id != vec2_type_) {
    last_error_msg_ =
        "OpTypeFunction: Parameter type was not defined or was not vec2.";
    return SPV_UNSUPPORTED;
  }

  static constexpr int kReturnTypeIndex = 1;
  uint32_t return_type = get_operand(inst, kReturnTypeIndex);
  if (return_type == 0 || return_type != vec4_type_) {
    last_error_msg_ =
        "OpTypeFunction: Return type was not defined or was not vec4.";
    return SPV_UNSUPPORTED;
  }

  main_function_type_ = inst->result_id;
  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleConstant(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kValueIndex = 2;

  if (inst->type_id == 0 || inst->type_id != float_type_) {
    last_error_msg_ = "OpConstant: Must have float-type.";
    return SPV_UNSUPPORTED;
  }

  float value = *reinterpret_cast<const float*>(get_literal(inst, kValueIndex));

  sksl_ << "const float " << ResolveName(inst->result_id) << " = " << value
        << ";\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleConstantComposite(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kValueIndex = 2;
  int opcount = inst->num_operands - kValueIndex;

  sksl_ << "const float" << opcount << " " << ResolveName(inst->result_id)
        << " = float" << opcount << "(";

  for (int i = 0; i < opcount; i++) {
    sksl_ << ResolveName(get_operand(inst, kValueIndex + i));
    if (i < opcount - 1) {
      sksl_ << ", ";
    }
  }

  sksl_ << ");\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleVariable(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kStorageClassIndex = 2;

  if (get_operand(inst, kStorageClassIndex) !=
      spv::StorageClassUniformConstant) {
    last_error_msg_ = "OpVariable: Must use storage class 'UniformConstant'";
    return SPV_UNSUPPORTED;
  }

  if (inst->type_id == 0 || (inst->type_id != float_uniform_type_ &&
                             inst->type_id != vec2_uniform_type_ &&
                             inst->type_id != vec3_uniform_type_ &&
                             inst->type_id != vec4_uniform_type_)) {
    last_error_msg_ = "OpVariable: Must use SSIR-valid type.";
    return SPV_UNSUPPORTED;
  }

  sksl_ << "uniform " << ResolveType(inst->type_id) << " "
        << ResolveName(inst->result_id) << ";\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleFunction(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kFunctionControlIndex = 2;
  static constexpr int kFunctionTypeIndex = 3;

  if (inst->result_id == 0 ||
      (inst->result_id != main_function_ &&
       imported_functions_.count(inst->result_id) == 0)) {
    last_error_msg_ =
        "OpFunction: Must be exported 'main' or imported function.";
    return SPV_UNSUPPORTED;
  }

  uint32_t function_control = get_operand(inst, kFunctionControlIndex);
  if (function_control != spv::FunctionControlMaskNone) {
    last_error_msg_ = "OpFunction: No function control flags are supported.";
    return SPV_UNSUPPORTED;
  }

  uint32_t function_type = get_operand(inst, kFunctionTypeIndex);
  if (function_type == 0 || function_type != main_function_type_) {
    last_error_msg_ = "OpFunction: Function type mismatch.";
    return SPV_UNSUPPORTED;
  }

  if (inst->type_id != vec4_type_) {
    last_error_msg_ = "OpFunction: Function must return vec4 type.";
    return SPV_UNSUPPORTED;
  }

  if (inst->result_id == main_function_) {
    sksl_ << "\nhalf4 main(";
  }

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleFunctionParameter(
    const spv_parsed_instruction_t* inst) {
  if (frag_position_param_ != 0) {
    last_error_msg_ =
        "OpFunctionParam: There can only be one specified parameter.";
    return SPV_UNSUPPORTED;
  }

  if (inst->type_id != vec2_type_) {
    last_error_msg_ = "OpFunctionParam: Param must be type vec2.";
    return SPV_UNSUPPORTED;
  }

  frag_position_param_ = inst->result_id;

  sksl_ << "float2 " << ResolveName(frag_position_param_);

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleFunctionCall(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kFunctionIdIndex = 2;
  static constexpr int kPosParamIndex = 3;
  static constexpr int kOperandCountWithOneParameter = 4;

  uint32_t function_id = get_operand(inst, kFunctionIdIndex);
  if (imported_functions_.count(function_id) == 0 ||
      inst->type_id != vec4_type_ ||
      inst->num_operands != kOperandCountWithOneParameter) {
    last_error_msg_ =
        "OpFunctionCall: Only imported vec4 fn(vec2) functions are supported";
    return SPV_UNSUPPORTED;
  }

  sksl_ << "  float4 " << ResolveName(inst->result_id) << " = sample("
        << imported_functions_[function_id] << ", "
        << ResolveName(get_operand(inst, kPosParamIndex)) << ");\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleLabel(const spv_parsed_instruction_t* inst) {
  if (last_op_ != spv::OpFunctionParameter) {
    last_error_msg_ =
        "OpLabel: The last instruction should have been OpFunctionParameter.";
    return SPV_UNSUPPORTED;
  }
  sksl_ << ") {\n";
  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleReturnValue(
    const spv_parsed_instruction_t* inst) {
  static constexpr int kReturnIdIndex = 0;
  if (return_ != 0) {
    last_error_msg_ = "OpReturnValue: There can only be one return value.";
    return SPV_UNSUPPORTED;
  }
  return_ = get_operand(inst, kReturnIdIndex);
  sksl_ << "  return half4(" << ResolveName(return_) << ");\n";
  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleLoad(const spv_parsed_instruction_t* inst) {
  std::string type = ResolveType(inst->type_id);
  if (type.empty()) {
    last_error_msg_ = "Invalid type.";
    return SPV_ERROR_INVALID_BINARY;
  }
  static constexpr int kPointerIndex = 2;
  sksl_ << "  " << type << " " << ResolveName(inst->result_id) << " = "
        << ResolveName(get_operand(inst, kPointerIndex)) << ";\n";
  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleFNegate(
    const spv_parsed_instruction_t* inst) {
  std::string type = ResolveType(inst->type_id);
  if (type.empty()) {
    last_error_msg_ = "Invalid type.";
    return SPV_ERROR_INVALID_BINARY;
  }
  sksl_ << "  " << type << " " << ResolveName(inst->result_id) << " = -"
        << ResolveName(get_operand(inst, 0)) << ";\n";
  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleOperator(
    const spv_parsed_instruction_t* inst,
    char op) {
  if (inst->num_operands != 4) {
    last_error_msg_ = "Operator '";
    last_error_msg_.push_back(op);
    last_error_msg_ += "' needs two arguments.";
    return SPV_ERROR_INVALID_BINARY;
  }
  std::string type = ResolveType(inst->type_id);
  if (type.empty()) {
    last_error_msg_ = "Invalid type.";
    return SPV_ERROR_INVALID_BINARY;
  }
  static constexpr int kFirstArgIndex = 2;
  sksl_ << "  " << type << " " << ResolveName(inst->result_id) << " = "
        << ResolveName(get_operand(inst, kFirstArgIndex)) << op
        << ResolveName(get_operand(inst, kFirstArgIndex + 1)) << ";\n";
  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleBuiltin(const spv_parsed_instruction_t* inst,
                                           std::string name) {
  if (inst->num_operands != 4) {
    last_error_msg_ = "Builtin '" + name + "' needs two arguments.";
    return SPV_ERROR_INVALID_BINARY;
  }
  std::string type = ResolveType(inst->type_id);
  if (type.empty()) {
    last_error_msg_ = "Invalid type.";
    return SPV_ERROR_INVALID_BINARY;
  }
  static constexpr int kFirstArgIndex = 2;
  sksl_ << "  " << type << " " << ResolveName(inst->result_id)
        << " = " + name + "(" << ResolveName(get_operand(inst, kFirstArgIndex))
        << ", " << ResolveName(get_operand(inst, kFirstArgIndex + 1)) << ");\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleExtInst(
    const spv_parsed_instruction_t* inst) {
  std::string type = ResolveType(inst->type_id);
  if (type.empty()) {
    last_error_msg_ = "Invalid type.";
    return SPV_ERROR_INVALID_BINARY;
  }

  if (inst->ext_inst_type != SPV_EXT_INST_TYPE_GLSL_STD_450) {
    last_error_msg_ = "OpExtInst: Must be from 'glsl.450.std'";
    return SPV_UNSUPPORTED;
  }

  static constexpr int kExtInstOperationIndex = 3;
  static constexpr int kExtInstFirstOperandIndex = 4;
  uint32_t glsl_op = get_operand(inst, kExtInstOperationIndex);
  std::string glsl_name = ResolveGLSLName(glsl_op);

  if (glsl_name.empty()) {
    last_error_msg_ = "OpExtInst: '" + std::to_string(glsl_op) +
                      "' is not a supported GLSL instruction.";
    return SPV_UNSUPPORTED;
  }

  sksl_ << "  " << type << " " << ResolveName(inst->result_id) << " = "
        << glsl_name << "(";

  int op_count = inst->num_operands - kExtInstFirstOperandIndex;
  for (int i = 0; i < op_count; i++) {
    sksl_ << ResolveName(get_operand(inst, kExtInstFirstOperandIndex + i));
    if (i != op_count - 1) {
      sksl_ << ", ";
    }
  }

  sksl_ << ");\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleCompositeConstruct(
    const spv_parsed_instruction_t* inst) {
  std::string type = ResolveType(inst->type_id);
  if (type.empty()) {
    last_error_msg_ = "Invalid type.";
    return SPV_ERROR_INVALID_BINARY;
  }

  sksl_ << "  " << type << " " << ResolveName(inst->result_id) << " = " << type
        << "(";

  static constexpr int kFirstComponentIndex = 2;
  const int component_count = inst->num_operands - kFirstComponentIndex;
  for (int i = 0; i < component_count; i++) {
    sksl_ << ResolveName(get_operand(inst, i + kFirstComponentIndex));
    if (i != component_count - 1) {
      sksl_ << ", ";
    }
  }
  sksl_ << ");\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleCompositeExtract(
    const spv_parsed_instruction_t* inst) {
  std::string type = ResolveType(inst->type_id);
  if (type.empty()) {
    last_error_msg_ = "Invalid type.";
    return SPV_ERROR_INVALID_BINARY;
  }

  static constexpr int kCompositeIndex = 2;
  static constexpr int kIndexIndex = 3;

  if (inst->num_operands > kIndexIndex + 1) {
    last_error_msg_ = "OpCompositeExtract: Only one index is supported.";
    return SPV_UNSUPPORTED;
  }

  sksl_ << "  " << type << " " << ResolveName(inst->result_id) << " = "
        << ResolveName(get_operand(inst, kCompositeIndex)) << '['
        << get_operand(inst, kIndexIndex) << "];\n";

  return SPV_SUCCESS;
}

spv_result_t TranspilerImpl::HandleFunctionEnd(
    const spv_parsed_instruction_t* inst) {
  sksl_ << "}\n";
  return SPV_SUCCESS;
}

std::string TranspilerImpl::ResolveGLSLName(uint32_t id) {
  switch (id) {
    case GLSLstd450Trunc:
      return "trunc";
    case GLSLstd450FAbs:
      return "abs";
    case GLSLstd450FSign:
      return "sign";
    case GLSLstd450Floor:
      return "floor";
    case GLSLstd450Ceil:
      return "ceil";
    case GLSLstd450Fract:
      return "fract";
    case GLSLstd450Radians:
      return "radians";
    case GLSLstd450Degrees:
      return "degrees";
    case GLSLstd450Sin:
      return "sin";
    case GLSLstd450Cos:
      return "cos";
    case GLSLstd450Tan:
      return "tan";
    case GLSLstd450Asin:
      return "asin";
    case GLSLstd450Acos:
      return "acos";
    case GLSLstd450Atan:
      return "atan";
    case GLSLstd450Atan2:
      return "atan2";
    case GLSLstd450Pow:
      return "pow";
    case GLSLstd450Exp:
      return "exp";
    case GLSLstd450Log:
      return "log";
    case GLSLstd450Exp2:
      return "exp2";
    case GLSLstd450Log2:
      return "log2";
    case GLSLstd450Sqrt:
      return "sqrt";
    case GLSLstd450InverseSqrt:
      return "inversesqrt";
    case GLSLstd450FMin:
      return "min";
    case GLSLstd450FMax:
      return "max";
    case GLSLstd450FClamp:
      return "clamp";
    case GLSLstd450FMix:
      return "mix";
    case GLSLstd450Step:
      return "step";
    case GLSLstd450SmoothStep:
      return "smoothstep";
    case GLSLstd450Length:
      return "length";
    case GLSLstd450Distance:
      return "distance";
    case GLSLstd450Cross:
      return "cross";
    case GLSLstd450Normalize:
      return "normalize";
    case GLSLstd450FaceForward:
      return "faceforward";
    case GLSLstd450Reflect:
      return "reflect";
    default:
      return "";
  }
}

}  // namespace spirv
}  // namespace flutter

