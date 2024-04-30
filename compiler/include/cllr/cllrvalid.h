
#pragma once

#include <algorithm>

#include "basic.h"

#include "cllrasm.h"
#include "error.h"

#define CLLR_VALID_FN(Name) ValidReason Name(ValidationLevel lvl, in<Instruction> i, in<Assembler> codeAsm)

#define CLLR_VALID_HAS_ID if (i.index == 0) return ValidReason::INVALID_NO_ID
#define CLLR_VALID_NO_ID if (i.index != 0) return ValidReason::INVALID_MISC

#define CLLR_VALID_HAS_OUT if (i.outType == 0 || !isType(codeAsm.getOp(i.outType))) return ValidReason::INVALID_OUT_TYPE
#define CLLR_VALID_NO_OUT if (i.outType != 0) return ValidReason::INVALID_OUT_TYPE
#define CLLR_VALID_OUT_TYPE(op) if (i.outType == 0 || (codeAsm.getOp(i.outType) != op)) return ValidReason::INVALID_OUT_TYPE

#define CLLR_VALID_TYPE(id) if (id == 0 || !isType(codeAsm.getOp(id))) return ValidReason::INVALID_TYPE
#define CLLR_VALID_VALUE(id) if (id == 0 || !isValue(codeAsm.getOp(id))) return ValidReason::INVALID_VALUE
#define CLLR_VALID_OPT_VALUE(id) if (id != 0 && !isValue(codeAsm.getOp(id))) return ValidReason::INVALID_VALUE
#define CLLR_VALID_LVALUE(id) if (id == 0 || !isLValue(codeAsm.getOp(id))) return ValidReason::INVALID_LVALUE
#define CLLR_VALID_VAR(id) if (id == 0 || !isVar(codeAsm.getOp(id))) return ValidReason::INVALID_VAR
#define CLLR_VALID_REF(id, op) if (id == 0 || codeAsm.getOp(id) != op) return ValidReason::INVALID_REF

#define CLLR_VALID_MAX_REFS(n) if ((MAX_REFS - std::count(i.refs.begin(), i.refs.end(), 0)) > n) { return ValidReason::INVALID_REF; }
#define CLLR_VALID_MAX_OPS(n) if ((MAX_OPS - std::count(i.operands.begin(), i.operands.end(), 0)) > n) { return ValidReason::INVALID_REF; }

#define CLLR_VALID_NO_REFS for (size_t off = 0; off < MAX_REFS; ++off) { if (i.refs[off] != 0) { return ValidReason::INVALID_REF; } }
#define CLLR_VALID_NO_OPS for (size_t off = 0; off < MAX_OPS; ++off) { if (i.operands[off] != 0) { return ValidReason::INVALID_OPERAND; } }

#define CLLR_VALID_OP_RANGE(n, min, max) if (n > max || n < min) return ValidReason::INVALID_OPERAND;
#define CLLR_VALID_OP_NONZERO(n) if (n == 0) return ValidReason::INVALID_OPERAND;

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

		using ValidFn = ValidReason(Validator::*)(ValidationLevel lvl, in<Instruction> i, in<Assembler> codeAsm);

		struct Validator
		{
			const uptr<ErrorHandler> errors;

			std::map<cllr::Opcode, ValidFn> validators;
			std::stack<SSA> scopes;

			sptr<const CompilerSettings> settings;

			Validator(sptr<const CompilerSettings> cs);

			bool validate(in<Assembler> codeAsm);

			//here we go again
			CLLR_VALID_FN(OpUnknown);

			CLLR_VALID_FN(OpShaderStage);
			CLLR_VALID_FN(OpShaderStageEnd);

			CLLR_VALID_FN(OpFunction);
			CLLR_VALID_FN(OpVarFuncArg);
			CLLR_VALID_FN(OpFunctionEnd);

			CLLR_VALID_FN(OpVarLocal);
			CLLR_VALID_FN(OpVarGlobal);
			CLLR_VALID_FN(OpVarShaderIn);
			CLLR_VALID_FN(OpVarShaderOut);
			CLLR_VALID_FN(OpVarDescriptor);

			CLLR_VALID_FN(OpCall);
			CLLR_VALID_FN(OpCallArg);

			CLLR_VALID_FN(OpTypeVoid);
			CLLR_VALID_FN(OpTypeFloat);
			CLLR_VALID_FN(OpTypeIntSign);
			CLLR_VALID_FN(OpTypeIntUnsign);
			CLLR_VALID_FN(OpTypeArray);
			CLLR_VALID_FN(OpTypeVector);
			CLLR_VALID_FN(OpTypeMatrix);
			CLLR_VALID_FN(OpTypeTexture);
			CLLR_VALID_FN(OpTypeStruct);

			CLLR_VALID_FN(OpTypeBool);
			CLLR_VALID_FN(OpTypePtr);
			CLLR_VALID_FN(OpTypeTuple);

			CLLR_VALID_FN(OpStructMember);
			CLLR_VALID_FN(OpStructEnd);

			CLLR_VALID_FN(OpScopeBegin);
			CLLR_VALID_FN(OpScopeEnd);

			CLLR_VALID_FN(OpLabel);
			CLLR_VALID_FN(OpJump);
			CLLR_VALID_FN(OpJumpCond);
			CLLR_VALID_FN(OpLoop);

			CLLR_VALID_FN(OpAssign);
			CLLR_VALID_FN(OpCompare);

			CLLR_VALID_FN(OpValueCast);
			CLLR_VALID_FN(OpValueConstruct);
			CLLR_VALID_FN(OpConstructArg);
			CLLR_VALID_FN(OpValueDeref);
			CLLR_VALID_FN(OpValueExpand);
			CLLR_VALID_FN(OpValueExpr);
			CLLR_VALID_FN(OpValueExprUnary);
			CLLR_VALID_FN(OpValueIntToFP);
			CLLR_VALID_FN(OpValueInvokePos);
			CLLR_VALID_FN(OpValueInvokeSize);
			CLLR_VALID_FN(OpValueLitArray);
			CLLR_VALID_FN(OpLitArrayElem);
			CLLR_VALID_FN(OpValueLitBool);
			CLLR_VALID_FN(OpValueLitFloat);
			CLLR_VALID_FN(OpValueLitInt);
			CLLR_VALID_FN(OpValueLitStr);
			CLLR_VALID_FN(OpValueMember);
			CLLR_VALID_FN(OpValueNull);
			CLLR_VALID_FN(OpValueReadVar);
			CLLR_VALID_FN(OpValueSample);
			CLLR_VALID_FN(OpValueSign);
			CLLR_VALID_FN(OpValueSubarray);
			CLLR_VALID_FN(OpValueUnsign);
			CLLR_VALID_FN(OpValueVecSwizzle);
			CLLR_VALID_FN(OpValueZero);

			CLLR_VALID_FN(OpReturn);
			CLLR_VALID_FN(OpReturnValue);
			CLLR_VALID_FN(OpDiscard);

		};

	}

}
