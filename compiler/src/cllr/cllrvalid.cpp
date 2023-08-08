
#include <algorithm>

#include "cllr/cllrvalid.h"

using namespace caliburn::cllr;

bool Validator::validate(ref<const InstructionVec> is)
{
	return true;
}

bool Validator::isType(Opcode op)
{
	const std::vector<Opcode> ops = {
		Opcode::TYPE_VOID,
		Opcode::TYPE_FLOAT,
		Opcode::TYPE_INT_SIGN,
		Opcode::TYPE_INT_UNSIGN,
		Opcode::TYPE_ARRAY,
		Opcode::TYPE_VECTOR,
		Opcode::TYPE_MATRIX,
		Opcode::TYPE_STRUCT,
		Opcode::TYPE_BOOL,
		Opcode::TYPE_PTR,
		Opcode::TYPE_TUPLE,
		//Opcode::TYPE_STRING
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

bool Validator::isValue(Opcode op)
{
	const std::vector<Opcode> ops = {
		Opcode::CALL,
		Opcode::COMPARE,
		Opcode::VALUE_CAST,
		Opcode::VALUE_DEREF,
		Opcode::VALUE_EXPR,
		Opcode::VALUE_EXPR_UNARY,
		Opcode::VALUE_INVOKE_POS,
		Opcode::VALUE_INVOKE_SIZE,
		Opcode::VALUE_LIT_ARRAY,
		Opcode::VALUE_LIT_BOOL,
		Opcode::VALUE_LIT_FP,
		Opcode::VALUE_LIT_INT,
		Opcode::VALUE_LIT_STR,
		Opcode::VALUE_MEMBER,
		Opcode::VALUE_NULL,
		Opcode::VALUE_READ_VAR,
		Opcode::VALUE_SUBARRAY,
		Opcode::VALUE_ZERO
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

bool Validator::isLValue(Opcode op)
{
	const std::vector<Opcode> ops = {
		Opcode::VALUE_DEREF,
		Opcode::VALUE_MEMBER,
		Opcode::VALUE_READ_VAR,
		Opcode::VALUE_SUBARRAY
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

bool Validator::isVar(Opcode op)
{
	const std::vector<Opcode> ops = {
		Opcode::VAR_LOCAL,
		Opcode::VAR_GLOBAL,
		Opcode::VAR_FUNC_ARG,
		Opcode::VAR_SHADER_IN,
		Opcode::VAR_SHADER_OUT,
		Opcode::VAR_DESCRIPTOR
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

CLLR_INSTRUCT_VALIDATE(Validator::OpUnknown)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpShaderStage)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpShaderStageEnd)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpFunction)
{
	if (i->index == 0)
	{
		return ValidReason::INVALID_NO_ID;
	}

	if (!isType(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpVarFuncArg)
{
	if (!isType(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpFunctionEnd)
{
	if (codeAsm.opFor(i->refs[0]) != Opcode::FUNCTION)
	{
		return ValidReason::INVALID_REF;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpVarLocal)
{
	if (!isType(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	if (i->refs[1] != 0)
	{
		if (!isValue(codeAsm.opFor(i->refs[1])))
		{
			return ValidReason::INVALID_VALUE;
		}

	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpVarGlobal)
{
	if (!isType(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	if (i->refs[1] == 0)
	{
		return ValidReason::INVALID_VALUE;
	}

	if (!isValue(codeAsm.opFor(i->refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpVarShaderIn)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpVarShaderOut)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpVarDescriptor)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpCall)
{
	auto fnCode = codeAsm.codeFor(i->refs[0]);

	if (fnCode->op != Opcode::FUNCTION)
	{
		return ValidReason::INVALID_REF;
	}

	//Checks if the call args match with the arguments set by the called function
	if (fnCode->operands[0] != i->operands[0])
	{
		return ValidReason::INVALID_MISC;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpCallArg)
{
	if (!isValue(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeVoid)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeFloat)
{
	auto const bits = i->operands[0];

	if (bits > caliburn::MAX_FLOAT_BITS || bits < caliburn::MIN_FLOAT_BITS)
	{
		return ValidReason::INVALID_OPERAND;
	}

	if (!IS_POW_2(bits))
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeIntSign)
{
	auto const bits = i->operands[0];

	if (bits > caliburn::MAX_INT_BITS || bits < caliburn::MIN_INT_BITS)
	{
		return ValidReason::INVALID_OPERAND;
	}

	if (!IS_POW_2(bits))
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeIntUnsign)
{
	auto const bits = i->operands[0];

	if (bits > caliburn::MAX_INT_BITS || bits < caliburn::MIN_INT_BITS)
	{
		return ValidReason::INVALID_OPERAND;
	}

	if (!IS_POW_2(bits))
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeArray)
{
	//length check
	if (i->operands[0] == 0)
	{
		return ValidReason::INVALID_OPERAND;
	}

	if (!isType(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeVector)
{
	if (!isType(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	auto const len = i->operands[0];

	if (len > caliburn::MAX_VECTOR_LEN || len < caliburn::MIN_VECTOR_LEN)
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeMatrix)
{
	//TODO consider matrix semantics

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeStruct)
{
	//empty structs are illegal
	if (i->operands[0] == 0)
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypeBool)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpTypePtr)
{
	if (!isType(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_REF;
	}

	return ValidReason::VALID;
}

//TOOD oops, I haven't begun to implement this yet
CLLR_INSTRUCT_VALIDATE(Validator::OpTypeTuple)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpStructMember)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpStructEnd)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpLabel)
{
	if (i->index == 0)
	{
		return ValidReason::INVALID_NO_ID;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpJump)
{
	if (codeAsm.opFor(i->refs[0]) != Opcode::LABEL)
	{
		return ValidReason::INVALID_REF;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpJumpCond)
{
	if (codeAsm.opFor(i->refs[0]) != Opcode::LABEL)
	{
		return ValidReason::INVALID_REF;
	}

	if (!isValue(codeAsm.opFor(i->refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpLoop)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpAssign)
{
	if (i->refs[0] == i->refs[1])
	{
		return ValidReason::INVALID_MISC;
	}

	if (!isLValue(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_LVALUE;
	}

	if (!isValue(codeAsm.opFor(i->refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpCompare)
{
	if (!isType(codeAsm.opFor(i->outType)))
	{
		return ValidReason::INVALID_OUT_TYPE;
	}

	if (!isValue(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	if (!isValue(codeAsm.opFor(i->refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueCast)
{
	if (!isType(codeAsm.opFor(i->outType)))
	{
		return ValidReason::INVALID_OUT_TYPE;
	}

	if (!isValue(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueConstruct)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpConstructArg)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueDeref)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueExpand)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueExpr)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueExprUnary)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueInvokePos)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueInvokeSize)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueLitArray)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpLitArrayElem)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueLitBool)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueLitFloat)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueLitInt)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueLitStr)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueMember)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueNull)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueReadVar)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueSign)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueSubarray)
{
	if (!isValue(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	if (!isValue(codeAsm.opFor(i->refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueUnsign)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpValueZero)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpReturn)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpReturnValue)
{
	if (!isValue(codeAsm.opFor(i->refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(Validator::OpDiscard)
{
	if (codeAsm.type != ShaderType::FRAGMENT)
	{
		return ValidReason::INVALID_CONTEXT;
	}

	return ValidReason::VALID;
}
