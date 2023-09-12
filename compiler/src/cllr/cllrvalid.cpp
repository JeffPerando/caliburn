
#include "cllr/cllrvalid.h"

#include <algorithm>

using namespace caliburn::cllr;

Validator::Validator(sptr<const CompilerSettings> cs) : settings(cs), validators({
	&valid::OpUnknown,

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
	bool err = false;

	if (lvl == ValidationLevel::NONE)
	{
		return true;
	}

	for (auto const& i : is)
	{
		auto const noteOp = lambda(sptr<Error> e)
		{
			e->note({ "Op:", std::string(OP_NAMES[(int)i.op])});
			e->note({ "ID:", std::to_string(i.index) });
			e->note({ "Out:", std::to_string(i.outType) });
			e->note({ "Operands: [", std::to_string(i.operands[0]), std::to_string(i.operands[1]), std::to_string(i.operands[2]), "]" });
			e->note({ "Refs: [", std::to_string(i.refs[0]), std::to_string(i.refs[1]), std::to_string(i.refs[2]), "]" });

		};

		if (lvl >= ValidationLevel::BASIC)
		{
			if (i.debugTkn == nullptr)
			{
				std::vector<std::string> msg;

				msg.push_back("CLLR instruction");
				
				if (i.index == 0)
				{
					msg.push_back(std::string(OP_NAMES[(int)i.op]));
				}
				else
				{
					msg.push_back(std::to_string(i.index));
				}

				msg.push_back("does not have a debug token");

				auto e = errors->err(msg, nullptr);

				e->note(codeGenErr);
				noteOp(e);
				err = true;

			}

			if (i.index != 0)
			{
				if (valid::isValue(i.op) && i.outType == 0)
				{
					auto e = errors->err("Value does not have an output type", i.debugTkn);

					e->note(codeGenErr);
					noteOp(e);
					err = true;

				}

				if (lvl == ValidationLevel::FULL)
				{
					for (int r = 0; r < 3; ++r)
					{
						if (i.refs[r] == i.index)
						{
							auto e = errors->err({ "CLLR instruction", std::to_string(i.index), "cannot reference itself" }, i.debugTkn);
							noteOp(e);
							err = true;

						}

					}

				}
				
			}

		}

		if (lvl < ValidationLevel::FULL)
		{
			//The full validation layer is quite slow
			//For now, we'll just not use it.
			continue;
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

		noteOp(e);
		err = true;

	}

	return !err;
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
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(0);
	
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpShaderStage)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

	//TODO validate shader stage

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpShaderStageEnd)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpFunction)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarFuncArg)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpFunctionEnd)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_OP(i.refs[0], Opcode::FUNCTION);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarLocal)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);
	CLLR_VALID_OPT_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarGlobal)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarShaderIn)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarShaderOut)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarDescriptor)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpCall)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	auto const& fnCode = codeAsm.codeFor(i.refs[0]);

	if (fnCode.op != Opcode::FUNCTION)
	{
		return ValidReason::INVALID_REF;
	}

	//Checks if the call args length equals the function's argument count
	if (fnCode.operands[0] != i.operands[0])
	{
		return ValidReason::INVALID_MISC;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpCallArg)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_REFS(2);
	CLLR_VALID_MAX_OPS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeVoid)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_REFS(0);
	CLLR_VALID_MAX_OPS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeFloat)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_REFS(0);

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
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

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
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

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
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);
	CLLR_VALID_TYPE(i.refs[0]);

	//length check
	if (i.operands[0] == 0)
	{
		return ValidReason::INVALID_OPERAND;
	}
	
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeVector)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	auto const len = i.operands[0];

	if (len > caliburn::MAX_VECTOR_LEN || len < caliburn::MIN_VECTOR_LEN)
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeMatrix)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	//TODO consider matrix semantics

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeStruct)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

	//empty structs are illegal
	if (i.operands[0] == 0)
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeBool)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypePtr)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

//TOOD oops, I haven't begun to implement this yet
CLLR_INSTRUCT_VALIDATE(valid::OpTypeTuple)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpStructMember)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);
	CLLR_VALID_OP(i.refs[1], Opcode::TYPE_STRUCT);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpStructEnd)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_OP(i.refs[0], Opcode::TYPE_STRUCT);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLabel)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpJump)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_OP(i.refs[0], Opcode::LABEL);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpJumpCond)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_OP(i.refs[0], Opcode::LABEL);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLoop)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	//TODO recall loop semantics
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpAssign)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_LVALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	//no self-assignment
	if (i.refs[0] == i.refs[1])
	{
		return ValidReason::INVALID_MISC;
	}
	
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpCompare)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueCast)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_TYPE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueConstruct)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpConstructArg)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueDeref)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueExpand)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueExpr)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueExprUnary)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueIntToFP)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueInvokePos)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueInvokeSize)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitArray)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLitArrayElem)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_OP(i.refs[0], Opcode::TYPE_ARRAY);
	CLLR_VALID_TYPE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitBool)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitFloat)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitInt)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitStr)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(0);
	//TODO string support

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueMember)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueNull)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueReadVar)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VAR(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueSign)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueSubarray)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);
	
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueUnsign)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueZero)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpReturn)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(0);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpReturnValue)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpDiscard)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(0);
	CLLR_VALID_MAX_REFS(0);

	if (codeAsm.type != ShaderType::FRAGMENT)
	{
		return ValidReason::INVALID_CONTEXT;
	}

	return ValidReason::VALID;
}
