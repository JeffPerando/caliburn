
#pragma once

#include <algorithm>

#include "basic.h"

#include "cllrasm.h"
#include "error.h"

#define CLLR_INSTRUCT_VALIDATE(Name) ValidReason Name(ValidationLevel lvl, in<Instruction> i, out<Assembler> codeAsm)

#define CLLR_VALID_HAS_ID if (i.index == 0) return ValidReason::INVALID_NO_ID
#define CLLR_VALID_NO_ID if (i.index != 0) return ValidReason::INVALID_MISC

#define CLLR_VALID_HAS_OUT if (i.outType == 0 || !isType(codeAsm.opFor(i.outType))) return ValidReason::INVALID_OUT_TYPE
#define CLLR_VALID_NO_OUT if (i.outType != 0) return ValidReason::INVALID_OUT_TYPE
#define CLLR_VALID_OUT_TYPE(op) if (i.outType == 0 || (codeAsm.opFor(i.outType) != op)) return ValidReason::INVALID_OUT_TYPE

#define CLLR_VALID_TYPE(id) if (id == 0 || !isType(codeAsm.opFor(id))) return ValidReason::INVALID_TYPE
#define CLLR_VALID_VALUE(id) if (id == 0 || !isValue(codeAsm.opFor(id))) return ValidReason::INVALID_VALUE
#define CLLR_VALID_OPT_VALUE(id) if (id != 0 && !isValue(codeAsm.opFor(id))) return ValidReason::INVALID_VALUE
#define CLLR_VALID_LVALUE(id) if (id == 0 || !isLValue(codeAsm.opFor(id))) return ValidReason::INVALID_LVALUE
#define CLLR_VALID_VAR(id) if (id == 0 || !isVar(codeAsm.opFor(id))) return ValidReason::INVALID_VAR
#define CLLR_VALID_REF(id, op) if (id == 0 || codeAsm.opFor(id) != op) return ValidReason::INVALID_REF

#define CLLR_VALID_MAX_REFS(n) if ((MAX_REFS - std::count(i.refs.begin(), i.refs.end(), 0)) > n) { return ValidReason::INVALID_REF; }
#define CLLR_VALID_MAX_OPS(n) if ((MAX_OPS - std::count(i.operands.begin(), i.operands.end(), 0)) > n) { return ValidReason::INVALID_REF; }

#define CLLR_VALID_NO_REFS for (size_t off = 0; off < MAX_REFS; ++off) { if (i.refs[off] != 0) { return ValidReason::INVALID_REF; } }
#define CLLR_VALID_NO_OPS for (size_t off = 0; off < MAX_OPS; ++off) { if (i.operands[off] != 0) { return ValidReason::INVALID_OPERAND; } }

#define CLLR_VALID_OP_RANGE(n, min, max) if (n > max || n < min) return ValidReason::INVALID_OPERAND;

namespace caliburn
{
	namespace cllr
	{
		struct Validator;

		static const std::vector<std::string> VALIDATION_REASONS = {
			"Valid",
			"Invalid Reference",
			"Invalid Operand",
			"No ID",
			"No Out Type",
			"Invalid Type",
			"Invalid Value",
			"Invalid LHS Value",
			"Invalid Variable",
			"Invalid Context",
			"Invalid Other"
		};

		enum class ValidReason
		{
			VALID,
			INVALID_REF,
			INVALID_OPERAND,
			INVALID_NO_ID,
			INVALID_OUT_TYPE,
			INVALID_TYPE,
			INVALID_VALUE,
			INVALID_LVALUE,
			INVALID_VAR,
			INVALID_CONTEXT,
			INVALID_MISC
		};

		using ValidFn = ValidReason(*)(ValidationLevel lvl, in<Instruction> i, out<Assembler> codeAsm);

		struct Validator
		{
			const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::CLLR_VALIDATION);
			const std::array<ValidFn, (uint64_t)cllr::Opcode::CLLR_OP_COUNT> validators;

			sptr<const CompilerSettings> settings;

			Validator(sptr<const CompilerSettings> cs);

			bool validate(ref<Assembler> codeAsm);

		};

		namespace valid
		{
			bool isType(cllr::Opcode op);

			bool isValue(cllr::Opcode op);

			bool isLValue(cllr::Opcode op);

			bool isVar(cllr::Opcode op);

			//here we go again
			CLLR_INSTRUCT_VALIDATE(OpUnknown);

			CLLR_INSTRUCT_VALIDATE(OpShaderStage);
			CLLR_INSTRUCT_VALIDATE(OpShaderStageEnd);

			CLLR_INSTRUCT_VALIDATE(OpFunction);
			CLLR_INSTRUCT_VALIDATE(OpVarFuncArg);
			CLLR_INSTRUCT_VALIDATE(OpFunctionEnd);

			CLLR_INSTRUCT_VALIDATE(OpVarLocal);
			CLLR_INSTRUCT_VALIDATE(OpVarGlobal);
			CLLR_INSTRUCT_VALIDATE(OpVarShaderIn);
			CLLR_INSTRUCT_VALIDATE(OpVarShaderOut);
			CLLR_INSTRUCT_VALIDATE(OpVarDescriptor);

			CLLR_INSTRUCT_VALIDATE(OpCall);
			CLLR_INSTRUCT_VALIDATE(OpCallArg);

			CLLR_INSTRUCT_VALIDATE(OpTypeVoid);
			CLLR_INSTRUCT_VALIDATE(OpTypeFloat);
			CLLR_INSTRUCT_VALIDATE(OpTypeIntSign);
			CLLR_INSTRUCT_VALIDATE(OpTypeIntUnsign);
			CLLR_INSTRUCT_VALIDATE(OpTypeArray);
			CLLR_INSTRUCT_VALIDATE(OpTypeVector);
			CLLR_INSTRUCT_VALIDATE(OpTypeMatrix);
			CLLR_INSTRUCT_VALIDATE(OpTypeStruct);

			CLLR_INSTRUCT_VALIDATE(OpTypeBool);
			CLLR_INSTRUCT_VALIDATE(OpTypePtr);
			CLLR_INSTRUCT_VALIDATE(OpTypeTuple);

			CLLR_INSTRUCT_VALIDATE(OpStructMember);
			CLLR_INSTRUCT_VALIDATE(OpStructEnd);

			CLLR_INSTRUCT_VALIDATE(OpLabel);
			CLLR_INSTRUCT_VALIDATE(OpJump);
			CLLR_INSTRUCT_VALIDATE(OpJumpCond);
			CLLR_INSTRUCT_VALIDATE(OpLoop);

			CLLR_INSTRUCT_VALIDATE(OpAssign);
			CLLR_INSTRUCT_VALIDATE(OpCompare);

			CLLR_INSTRUCT_VALIDATE(OpValueCast);
			CLLR_INSTRUCT_VALIDATE(OpValueConstruct);
			CLLR_INSTRUCT_VALIDATE(OpConstructArg);
			CLLR_INSTRUCT_VALIDATE(OpValueDeref);
			CLLR_INSTRUCT_VALIDATE(OpValueExpand);
			CLLR_INSTRUCT_VALIDATE(OpValueExpr);
			CLLR_INSTRUCT_VALIDATE(OpValueExprUnary);
			CLLR_INSTRUCT_VALIDATE(OpValueIntToFP);
			CLLR_INSTRUCT_VALIDATE(OpValueInvokePos);
			CLLR_INSTRUCT_VALIDATE(OpValueInvokeSize);
			CLLR_INSTRUCT_VALIDATE(OpValueLitArray);
			CLLR_INSTRUCT_VALIDATE(OpLitArrayElem);
			CLLR_INSTRUCT_VALIDATE(OpValueLitBool);
			CLLR_INSTRUCT_VALIDATE(OpValueLitFloat);
			CLLR_INSTRUCT_VALIDATE(OpValueLitInt);
			CLLR_INSTRUCT_VALIDATE(OpValueLitStr);
			CLLR_INSTRUCT_VALIDATE(OpValueMember);
			CLLR_INSTRUCT_VALIDATE(OpValueNull);
			CLLR_INSTRUCT_VALIDATE(OpValueReadVar);
			CLLR_INSTRUCT_VALIDATE(OpValueSign);
			CLLR_INSTRUCT_VALIDATE(OpValueSubarray);
			CLLR_INSTRUCT_VALIDATE(OpValueUnsign);
			CLLR_INSTRUCT_VALIDATE(OpValueVecSwizzle);
			CLLR_INSTRUCT_VALIDATE(OpValueZero);

			CLLR_INSTRUCT_VALIDATE(OpReturn);
			CLLR_INSTRUCT_VALIDATE(OpReturnValue);
			CLLR_INSTRUCT_VALIDATE(OpDiscard);

		}

	}

}
