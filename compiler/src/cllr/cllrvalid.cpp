
#include "cllr/cllrvalid.h"

#include <algorithm>

using namespace caliburn::cllr;

Validator::Validator(sptr<const CompilerSettings> cs) : settings(cs), validators({
	&valid::OpShaderStage,
	&valid::OpShaderStageEnd,
	&valid::OpFunction,
	&valid::OpVarFuncArg,
	&valid::OpFunctionEnd,

	&valid::OpVarLocal,
	&valid::OpVarGlobal,
	&valid::OpVarShaderIn,
	&valid::OpVarShaderOut,
	&valid::OpVarDescriptor,

	&valid::OpCall,
	&valid::OpCallArg,

	&valid::OpTypeVoid,
	&valid::OpTypeFloat,
	&valid::OpTypeIntSign,
	&valid::OpTypeIntUnsign,
	&valid::OpTypeArray,
	&valid::OpTypeVector,
	&valid::OpTypeMatrix,

	&valid::OpTypeStruct,
	&valid::OpStructMember,
	&valid::OpStructEnd,

	&valid::OpTypeBool,
	&valid::OpTypePtr,
	&valid::OpTypeTuple,

	&valid::OpLabel,
	&valid::OpJump,
	&valid::OpJumpCond,

	&valid::OpAssign,
	&valid::OpCompare,

	&valid::OpValueCast,
	&valid::OpValueConstruct,
	&valid::OpConstructArg,
	&valid::OpValueDeref,
	&valid::OpValueExpand,
	&valid::OpValueExpr,
	&valid::OpValueExprUnary,
	&valid::OpValueIntToFP,
	&valid::OpValueInvokePos,
	&valid::OpValueInvokeSize,
	&valid::OpValueLitArray,
	&valid::OpLitArrayElem,
	&valid::OpValueLitBool,
	&valid::OpValueLitFloat,
	&valid::OpValueLitInt,
	&valid::OpValueLitStr,
	&valid::OpValueMember,
	&valid::OpValueNull,
	&valid::OpValueReadVar,
	&valid::OpValueSign,
	&valid::OpValueSubarray,
	&valid::OpValueUnsign,
	&valid::OpValueZero,

	&valid::OpReturn,
	&valid::OpReturnValue,
	&valid::OpDiscard

}) {}

bool Validator::validate(ref<Assembler> codeAsm)
{
	auto const codeGenErr = "This is generally a code generation error. You should file an issue on the official Caliburn GitHub repo";
	auto const& is = codeAsm.getCode();
	auto const lvl = settings->vLvl;

	if (lvl == ValidationLevel::NONE)
	{
		return true;
	}

	for (auto const& i : is)
	{
		if (lvl >= ValidationLevel::BASIC)
		{
			if (i.debugTkn == nullptr)
			{
				auto e = errors->err({ "CLLR instruction", std::to_string(i.index), "does not have a debug token" }, nullptr);

				e->note(codeGenErr);

			}

			if (i.index != 0)
			{
				if (valid::isValue(i.op) && i.outType == 0)
				{
					auto e = errors->err("Value does not have an output type", i.debugTkn);

					e->note(codeGenErr);

				}

				if (lvl == ValidationLevel::FULL)
				{
					for (int r = 0; r < 3; ++r)
					{
						if (i.refs[r] == i.index)
						{
							auto e = errors->err({ "CLLR instruction", std::to_string(i.index), "cannot reference itself" }, i.debugTkn);

						}

					}

				}
				
			}

		}

		auto const reason = validators[(int)i.op](lvl, i, codeAsm);

		if (reason == ValidReason::VALID)
		{
			continue;
		}

		auto e = errors->err({ VALIDATION_REASONS[(int)reason] }, i.debugTkn);

		if (reason == ValidReason::INVALID_NO_ID)
		{
			e->note("This opcode requires a unique ID. This is a codegen error. Please file an issue on the official Caliburn GitHub repo");
		}
		else if (reason == ValidReason::INVALID_VAR)
		{
			e->note("Check to see if the debug token is pointing to a variable or not");
		}
		else if (reason == ValidReason::INVALID_LVALUE)
		{
			e->note("One of the references has to be an lvalue.");
		}
		
		e->note({ "ID:", std::to_string(i.index) });
		e->note({ "Out:", std::to_string(i.outType) });
		e->note({ "Operands: [", std::to_string(i.operands[0]), std::to_string(i.operands[1]), std::to_string(i.operands[2]), "]" });
		e->note({ "Refs: [", std::to_string(i.refs[0]), std::to_string(i.refs[1]), std::to_string(i.refs[2]), "]" });

	}

	return true;
}

bool valid::isType(Opcode op)
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

bool valid::isValue(Opcode op)
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

bool valid::isLValue(Opcode op)
{
	const std::vector<Opcode> ops = {
		Opcode::VALUE_DEREF,
		Opcode::VALUE_MEMBER,
		Opcode::VALUE_READ_VAR,
		Opcode::VALUE_SUBARRAY
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

bool valid::isVar(Opcode op)
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

CLLR_INSTRUCT_VALIDATE(valid::OpUnknown)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpShaderStage)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpShaderStageEnd)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpFunction)
{
	if (i.index == 0)
	{
		return ValidReason::INVALID_NO_ID;
	}

	if (!isType(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarFuncArg)
{
	if (!isType(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpFunctionEnd)
{
	if (codeAsm.opFor(i.refs[0]) != Opcode::FUNCTION)
	{
		return ValidReason::INVALID_REF;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarLocal)
{
	if (!isType(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	if (i.refs[1] != 0)
	{
		if (!isValue(codeAsm.opFor(i.refs[1])))
		{
			return ValidReason::INVALID_VALUE;
		}

	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarGlobal)
{
	if (!isType(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	if (i.refs[1] == 0)
	{
		return ValidReason::INVALID_VALUE;
	}

	if (!isValue(codeAsm.opFor(i.refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarShaderIn)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarShaderOut)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarDescriptor)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpCall)
{
	auto const& fnCode = codeAsm.codeFor(i.refs[0]);

	if (fnCode.op != Opcode::FUNCTION)
	{
		return ValidReason::INVALID_REF;
	}

	//Checks if the call args match with the arguments set by the called function
	if (fnCode.operands[0] != i.operands[0])
	{
		return ValidReason::INVALID_MISC;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpCallArg)
{
	if (!isValue(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeVoid)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeFloat)
{
	auto const bits = i.operands[0];

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

CLLR_INSTRUCT_VALIDATE(valid::OpTypeIntSign)
{
	auto const bits = i.operands[0];

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

CLLR_INSTRUCT_VALIDATE(valid::OpTypeIntUnsign)
{
	auto const bits = i.operands[0];

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

CLLR_INSTRUCT_VALIDATE(valid::OpTypeArray)
{
	//length check
	if (i.operands[0] == 0)
	{
		return ValidReason::INVALID_OPERAND;
	}

	if (!isType(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeVector)
{
	if (!isType(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_TYPE;
	}

	auto const len = i.operands[0];

	if (len > caliburn::MAX_VECTOR_LEN || len < caliburn::MIN_VECTOR_LEN)
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeMatrix)
{
	//TODO consider matrix semantics

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeStruct)
{
	//empty structs are illegal
	if (i.operands[0] == 0)
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeBool)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypePtr)
{
	if (!isType(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_REF;
	}

	return ValidReason::VALID;
}

//TOOD oops, I haven't begun to implement this yet
CLLR_INSTRUCT_VALIDATE(valid::OpTypeTuple)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpStructMember)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpStructEnd)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLabel)
{
	if (i.index == 0)
	{
		return ValidReason::INVALID_NO_ID;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpJump)
{
	if (codeAsm.opFor(i.refs[0]) != Opcode::LABEL)
	{
		return ValidReason::INVALID_REF;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpJumpCond)
{
	if (codeAsm.opFor(i.refs[0]) != Opcode::LABEL)
	{
		return ValidReason::INVALID_REF;
	}

	if (!isValue(codeAsm.opFor(i.refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLoop)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpAssign)
{
	if (i.refs[0] == i.refs[1])
	{
		return ValidReason::INVALID_MISC;
	}

	if (!isLValue(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_LVALUE;
	}

	if (!isValue(codeAsm.opFor(i.refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpCompare)
{
	if (!isType(codeAsm.opFor(i.outType)))
	{
		return ValidReason::INVALID_OUT_TYPE;
	}

	if (!isValue(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	if (!isValue(codeAsm.opFor(i.refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueCast)
{
	if (!isType(codeAsm.opFor(i.outType)))
	{
		return ValidReason::INVALID_OUT_TYPE;
	}

	if (!isValue(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueConstruct)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpConstructArg)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueDeref)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueExpand)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueExpr)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueExprUnary)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueIntToFP)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueInvokePos)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueInvokeSize)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitArray)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLitArrayElem)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitBool)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitFloat)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitInt)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitStr)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueMember)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueNull)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueReadVar)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueSign)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueSubarray)
{
	if (!isValue(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	if (!isValue(codeAsm.opFor(i.refs[1])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueUnsign)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueZero)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpReturn)
{
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpReturnValue)
{
	if (!isValue(codeAsm.opFor(i.refs[0])))
	{
		return ValidReason::INVALID_VALUE;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpDiscard)
{
	if (codeAsm.type != ShaderType::FRAGMENT)
	{
		return ValidReason::INVALID_CONTEXT;
	}

	return ValidReason::VALID;
}
