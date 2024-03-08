
#include "cllr/cllrvalid.h"

#include <algorithm>
#include <iostream>

using namespace caliburn::cllr;

Validator::Validator(sptr<const CompilerSettings> cs) : settings(cs), errors(new_uptr<ErrorHandler>(CompileStage::CLLR_VALIDATION, cs))
{
	validators.emplace(Opcode::UNKNOWN, &valid::OpUnknown);

	validators.emplace(Opcode::SHADER_STAGE, &valid::OpShaderStage);
	validators.emplace(Opcode::SHADER_STAGE_END, &valid::OpShaderStageEnd);
	
	validators.emplace(Opcode::FUNCTION, &valid::OpFunction);
	validators.emplace(Opcode::VAR_FUNC_ARG, &valid::OpVarFuncArg);
	validators.emplace(Opcode::FUNCTION_END, &valid::OpFunctionEnd);

	validators.emplace(Opcode::VAR_LOCAL, &valid::OpVarLocal);
	validators.emplace(Opcode::VAR_GLOBAL, &valid::OpVarGlobal);
	validators.emplace(Opcode::VAR_SHADER_IN, &valid::OpVarShaderIn);
	validators.emplace(Opcode::VAR_SHADER_OUT, &valid::OpVarShaderOut);
	validators.emplace(Opcode::VAR_DESCRIPTOR, &valid::OpVarDescriptor);

	validators.emplace(Opcode::CALL, &valid::OpCall);
	validators.emplace(Opcode::CALL_ARG, &valid::OpCallArg);

	validators.emplace(Opcode::TYPE_VOID, &valid::OpTypeVoid);
	validators.emplace(Opcode::TYPE_FLOAT, &valid::OpTypeFloat);
	validators.emplace(Opcode::TYPE_INT_SIGN, &valid::OpTypeIntSign);
	validators.emplace(Opcode::TYPE_INT_UNSIGN, &valid::OpTypeIntUnsign);
	validators.emplace(Opcode::TYPE_ARRAY, &valid::OpTypeArray);
	validators.emplace(Opcode::TYPE_VECTOR, &valid::OpTypeVector);
	validators.emplace(Opcode::TYPE_MATRIX, &valid::OpTypeMatrix);
	//validators.emplace(Opcode::TYPE_TEXTURE, &valid::OpTypeMatrix);

	validators.emplace(Opcode::TYPE_STRUCT, &valid::OpTypeStruct);
	validators.emplace(Opcode::STRUCT_MEMBER, &valid::OpStructMember);
	validators.emplace(Opcode::STRUCT_END, &valid::OpStructEnd);

	validators.emplace(Opcode::TYPE_BOOL, &valid::OpTypeBool);
	validators.emplace(Opcode::TYPE_PTR, &valid::OpTypePtr);
	validators.emplace(Opcode::TYPE_TUPLE, &valid::OpTypeTuple);

	validators.emplace(Opcode::LABEL, &valid::OpLabel);
	validators.emplace(Opcode::JUMP, &valid::OpJump);
	validators.emplace(Opcode::JUMP_COND, &valid::OpJumpCond);
	validators.emplace(Opcode::LOOP, &valid::OpLoop);

	validators.emplace(Opcode::ASSIGN, &valid::OpAssign);
	validators.emplace(Opcode::COMPARE, &valid::OpCompare);

	validators.emplace(Opcode::VALUE_CAST, &valid::OpValueCast);
	validators.emplace(Opcode::VALUE_CONSTRUCT, &valid::OpValueConstruct);
	validators.emplace(Opcode::CONSTRUCT_ARG, &valid::OpConstructArg);
	validators.emplace(Opcode::VALUE_DEREF, &valid::OpValueDeref);
	validators.emplace(Opcode::VALUE_EXPAND, &valid::OpValueExpand);
	validators.emplace(Opcode::VALUE_EXPR, &valid::OpValueExpr);
	validators.emplace(Opcode::VALUE_EXPR_UNARY, &valid::OpValueExprUnary);
	validators.emplace(Opcode::VALUE_INT_TO_FP, &valid::OpValueIntToFP);
	validators.emplace(Opcode::VALUE_INVOKE_POS, &valid::OpValueInvokePos);
	validators.emplace(Opcode::VALUE_INVOKE_SIZE, &valid::OpValueInvokeSize);
	validators.emplace(Opcode::VALUE_LIT_ARRAY, &valid::OpValueLitArray);
	validators.emplace(Opcode::LIT_ARRAY_ELEM, &valid::OpLitArrayElem);
	validators.emplace(Opcode::VALUE_LIT_BOOL, &valid::OpValueLitBool);
	validators.emplace(Opcode::VALUE_LIT_FP, &valid::OpValueLitFloat);
	validators.emplace(Opcode::VALUE_LIT_INT, &valid::OpValueLitInt);
	validators.emplace(Opcode::VALUE_LIT_STR, &valid::OpValueLitStr);
	validators.emplace(Opcode::VALUE_MEMBER, &valid::OpValueMember);
	validators.emplace(Opcode::VALUE_NULL, &valid::OpValueNull);
	validators.emplace(Opcode::VALUE_READ_VAR, &valid::OpValueReadVar);
	validators.emplace(Opcode::VALUE_SIGN, &valid::OpValueSign);
	validators.emplace(Opcode::VALUE_SUBARRAY, &valid::OpValueSubarray);
	validators.emplace(Opcode::VALUE_UNSIGN, &valid::OpValueUnsign);
	validators.emplace(Opcode::VALUE_VEC_SWIZZLE, &valid::OpValueVecSwizzle);
	validators.emplace(Opcode::VALUE_ZERO, &valid::OpValueZero);

	validators.emplace(Opcode::RETURN, &valid::OpReturn);
	validators.emplace(Opcode::RETURN_VALUE, &valid::OpReturnValue);
	validators.emplace(Opcode::DISCARD, &valid::OpDiscard);

}

bool Validator::validate(in<Assembler> codeAsm)
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
		if (i.index != 0)
		{
			if (isValue(i.op) && i.outType == 0)
			{
				auto e = errors->err("Value does not have an output type", i.debugTkn);

				e->note(codeGenErr);
				e->note(i.toStr());

			}

			if (lvl == ValidationLevel::FULL)
			{
				for (int r = 0; r < 3; ++r)
				{
					if (i.refs[r] == i.index)
					{
						auto e = errors->err({ "CLLR instruction", std::to_string(i.index), "cannot reference itself" }, i.debugTkn);
						e->note(i.toStr());

					}

				}

			}

		}

		if (lvl < ValidationLevel::FULL)
		{
			continue;
		}

		try
		{
			if (auto fn = validators.find(i.op); fn != validators.end())
			{
				auto const reason = (*fn->second)(lvl, i, codeAsm);

				if (reason != ValidReason::VALID)
				{
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

					e->note(i.toStr());

				}

			}
			else
			{
				std::cout << "Op " << SCAST<int>(i.op) << " does not have a validator function\n";
			}
			
		}
		catch (std::exception e)
		{
			std::cout << e.what() << '\n';
		}

		if (lvl != ValidationLevel::DEV)
		{
			continue;
		}

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
			e->note(i.toStr());
			
		}

	}

	return errors->empty();
}

CLLR_INSTRUCT_VALIDATE(valid::OpUnknown)
{
	CLLR_VALID_OP(Opcode::UNKNOWN);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;
	
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpShaderStage)
{
	CLLR_VALID_OP(Opcode::SHADER_STAGE);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	//TODO validate shader stage

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpShaderStageEnd)
{
	CLLR_VALID_OP(Opcode::SHADER_STAGE_END);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpFunction)
{
	CLLR_VALID_OP(Opcode::FUNCTION);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarFuncArg)
{
	CLLR_VALID_OP(Opcode::VAR_FUNC_ARG);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpFunctionEnd)
{
	CLLR_VALID_OP(Opcode::FUNCTION_END);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_REF(i.refs[0], Opcode::FUNCTION);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarLocal)
{
	CLLR_VALID_OP(Opcode::VAR_LOCAL);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);
	CLLR_VALID_OPT_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarGlobal)
{
	CLLR_VALID_OP(Opcode::VAR_GLOBAL);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarShaderIn)
{
	CLLR_VALID_OP(Opcode::VAR_SHADER_IN);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarShaderOut)
{
	CLLR_VALID_OP(Opcode::VAR_SHADER_OUT);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpVarDescriptor)
{
	CLLR_VALID_OP(Opcode::VAR_DESCRIPTOR);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpCall)
{
	CLLR_VALID_OP(Opcode::CALL);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	auto const& fnCode = codeAsm.getIns(i.refs[0]);

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
	CLLR_VALID_OP(Opcode::CALL_ARG);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_REFS(2);
	CLLR_VALID_MAX_OPS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeVoid)
{
	CLLR_VALID_OP(Opcode::TYPE_VOID);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_REFS;
	CLLR_VALID_NO_OPS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeFloat)
{
	CLLR_VALID_OP(Opcode::TYPE_FLOAT);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_REFS;

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
	CLLR_VALID_OP(Opcode::TYPE_INT_SIGN);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

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
	CLLR_VALID_OP(Opcode::TYPE_INT_UNSIGN);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

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
	CLLR_VALID_OP(Opcode::TYPE_ARRAY);
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
	CLLR_VALID_OP(Opcode::TYPE_VECTOR);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	CLLR_VALID_OP_RANGE(i.operands[0], caliburn::MIN_VECTOR_LEN, caliburn::MAX_VECTOR_LEN);
	
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeMatrix)
{
	CLLR_VALID_OP(Opcode::TYPE_MATRIX);
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
	CLLR_VALID_OP(Opcode::TYPE_STRUCT);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	//empty structs are illegal
	if (i.operands[0] == 0)
	{
		return ValidReason::INVALID_OPERAND;
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypeBool)
{
	CLLR_VALID_OP(Opcode::TYPE_BOOL);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpTypePtr)
{
	CLLR_VALID_OP(Opcode::TYPE_PTR);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

//TOOD oops, I haven't begun to implement this yet
CLLR_INSTRUCT_VALIDATE(valid::OpTypeTuple)
{
	CLLR_VALID_OP(Opcode::TYPE_TUPLE);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpStructMember)
{
	CLLR_VALID_OP(Opcode::STRUCT_MEMBER);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);
	CLLR_VALID_REF(i.refs[1], Opcode::TYPE_STRUCT);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpStructEnd)
{
	CLLR_VALID_OP(Opcode::STRUCT_END);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_REF(i.refs[0], Opcode::TYPE_STRUCT);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLabel)
{
	CLLR_VALID_OP(Opcode::LABEL);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpJump)
{
	CLLR_VALID_OP(Opcode::JUMP);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_REF(i.refs[0], Opcode::LABEL);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpJumpCond)
{
	CLLR_VALID_OP(Opcode::JUMP_COND);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_REF(i.refs[0], Opcode::LABEL);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLoop)
{
	CLLR_VALID_OP(Opcode::LOOP);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	//TODO recall loop semantics
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpAssign)
{
	CLLR_VALID_OP(Opcode::ASSIGN);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
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
	CLLR_VALID_OP(Opcode::COMPARE);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueCast)
{
	CLLR_VALID_OP(Opcode::VALUE_CAST);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_TYPE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueConstruct)
{
	CLLR_VALID_OP(Opcode::VALUE_CONSTRUCT);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpConstructArg)
{
	CLLR_VALID_OP(Opcode::CONSTRUCT_ARG);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueDeref)
{
	CLLR_VALID_OP(Opcode::VALUE_DEREF);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueExpand)
{
	CLLR_VALID_OP(Opcode::VALUE_EXPAND);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueExpr)
{
	CLLR_VALID_OP(Opcode::VALUE_EXPR);
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
	CLLR_VALID_OP(Opcode::VALUE_EXPR_UNARY);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueIntToFP)
{
	CLLR_VALID_OP(Opcode::VALUE_INT_TO_FP);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueInvokePos)
{
	CLLR_VALID_OP(Opcode::VALUE_INVOKE_POS);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueInvokeSize)
{
	CLLR_VALID_OP(Opcode::VALUE_INVOKE_SIZE);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitArray)
{
	CLLR_VALID_OP(Opcode::VALUE_LIT_ARRAY);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpLitArrayElem)
{
	CLLR_VALID_OP(Opcode::LIT_ARRAY_ELEM);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_REF(i.refs[0], Opcode::TYPE_ARRAY);
	CLLR_VALID_TYPE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitBool)
{
	CLLR_VALID_OP(Opcode::VALUE_LIT_BOOL);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitFloat)
{
	CLLR_VALID_OP(Opcode::VALUE_LIT_FP);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitInt)
{
	CLLR_VALID_OP(Opcode::VALUE_LIT_INT);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueLitStr)
{
	CLLR_VALID_OP(Opcode::VALUE_LIT_STR);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;
	//TODO string support

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueMember)
{
	CLLR_VALID_OP(Opcode::VALUE_MEMBER);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueNull)
{
	CLLR_VALID_OP(Opcode::VALUE_NULL);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueReadVar)
{
	CLLR_VALID_OP(Opcode::VALUE_READ_VAR);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VAR(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueSign)
{
	CLLR_VALID_OP(Opcode::VALUE_SIGN);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueSubarray)
{
	CLLR_VALID_OP(Opcode::VALUE_SUBARRAY);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);
	
	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueUnsign)
{
	CLLR_VALID_OP(Opcode::VALUE_UNSIGN);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueVecSwizzle)
{
	CLLR_VALID_OP(Opcode::VALUE_VEC_SWIZZLE);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_OUT_TYPE(Opcode::TYPE_VECTOR);
	CLLR_VALID_MAX_REFS(1);
	CLLR_VALID_VALUE(i.refs[0]);

	const uint32_t outVecLen = codeAsm.getIns(i.outType).operands[0];

	for (auto x = 0; x < MAX_OPS; ++x)
	{
		if (i.operands[x] >= outVecLen)
		{
			return ValidReason::INVALID_OPERAND;
		}
	}

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpValueZero)
{
	CLLR_VALID_OP(Opcode::VALUE_ZERO);
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpReturn)
{
	CLLR_VALID_OP(Opcode::RETURN);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpReturnValue)
{
	CLLR_VALID_OP(Opcode::RETURN_VALUE);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_INSTRUCT_VALIDATE(valid::OpDiscard)
{
	CLLR_VALID_OP(Opcode::DISCARD);
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	if (codeAsm.type != ShaderType::FRAGMENT)
	{
		return ValidReason::INVALID_CONTEXT;
	}

	return ValidReason::VALID;
}
