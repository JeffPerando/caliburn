
#include "cllr/cllrvalid.h"

#include <algorithm>
#include <iostream>

using namespace caliburn::cllr;

#define CLLR_VALID_IMPL(Name) ValidReason Validator::Name(ValidationLevel lvl, in<Instruction> i, in<Assembler> codeAsm)

Validator::Validator(sptr<const CompilerSettings> cs) : settings(cs), errors(new_uptr<ErrorHandler>(CompileStage::CLLR_VALIDATION, cs))
{
	validators.emplace(Opcode::NO_OP, &Validator::OpNoop);

	validators.emplace(Opcode::SHADER_STAGE, &Validator::OpShaderStage);
	validators.emplace(Opcode::SHADER_STAGE_END, &Validator::OpShaderStageEnd);

	validators.emplace(Opcode::FUNCTION, &Validator::OpFunction);
	validators.emplace(Opcode::VAR_FUNC_ARG, &Validator::OpVarFuncArg);
	validators.emplace(Opcode::FUNCTION_END, &Validator::OpFunctionEnd);

	validators.emplace(Opcode::VAR_LOCAL, &Validator::OpVarLocal);
	validators.emplace(Opcode::VAR_GLOBAL, &Validator::OpVarGlobal);
	validators.emplace(Opcode::VAR_SHADER_IN, &Validator::OpVarShaderIn);
	validators.emplace(Opcode::VAR_SHADER_OUT, &Validator::OpVarShaderOut);
	validators.emplace(Opcode::VAR_DESCRIPTOR, &Validator::OpVarDescriptor);

	validators.emplace(Opcode::CALL, &Validator::OpCall);
	validators.emplace(Opcode::CALL_ARG, &Validator::OpCallArg);

	validators.emplace(Opcode::TYPE_VOID, &Validator::OpTypeVoid);
	validators.emplace(Opcode::TYPE_FLOAT, &Validator::OpTypeFloat);
	validators.emplace(Opcode::TYPE_INT_SIGN, &Validator::OpTypeIntSign);
	validators.emplace(Opcode::TYPE_INT_UNSIGN, &Validator::OpTypeIntUnsign);
	validators.emplace(Opcode::TYPE_ARRAY, &Validator::OpTypeArray);
	validators.emplace(Opcode::TYPE_VECTOR, &Validator::OpTypeVector);
	validators.emplace(Opcode::TYPE_MATRIX, &Validator::OpTypeMatrix);
	validators.emplace(Opcode::TYPE_TEXTURE, &Validator::OpTypeTexture);

	validators.emplace(Opcode::TYPE_STRUCT, &Validator::OpTypeStruct);
	validators.emplace(Opcode::STRUCT_MEMBER, &Validator::OpStructMember);
	validators.emplace(Opcode::STRUCT_END, &Validator::OpStructEnd);

	validators.emplace(Opcode::TYPE_BOOL, &Validator::OpTypeBool);
	validators.emplace(Opcode::TYPE_PTR, &Validator::OpTypePtr);
	validators.emplace(Opcode::TYPE_TUPLE, &Validator::OpTypeTuple);

	validators.emplace(Opcode::SCOPE_BEGIN, &Validator::OpScopeBegin);
	validators.emplace(Opcode::SCOPE_END, &Validator::OpScopeEnd);

	validators.emplace(Opcode::LABEL, &Validator::OpLabel);
	validators.emplace(Opcode::JUMP, &Validator::OpJump);
	validators.emplace(Opcode::JUMP_COND, &Validator::OpJumpCond);
	validators.emplace(Opcode::LOOP, &Validator::OpLoop);

	validators.emplace(Opcode::ASSIGN, &Validator::OpAssign);
	validators.emplace(Opcode::COMPARE, &Validator::OpCompare);

	validators.emplace(Opcode::VALUE_CAST, &Validator::OpValueCast);
	validators.emplace(Opcode::VALUE_CONSTRUCT, &Validator::OpValueConstruct);
	validators.emplace(Opcode::CONSTRUCT_ARG, &Validator::OpConstructArg);
	validators.emplace(Opcode::VALUE_DEREF, &Validator::OpValueDeref);
	validators.emplace(Opcode::VALUE_EXPAND, &Validator::OpValueExpand);
	validators.emplace(Opcode::VALUE_EXPR, &Validator::OpValueExpr);
	validators.emplace(Opcode::VALUE_EXPR_UNARY, &Validator::OpValueExprUnary);
	validators.emplace(Opcode::VALUE_INT_TO_FP, &Validator::OpValueIntToFP);
	validators.emplace(Opcode::VALUE_INVOKE_POS, &Validator::OpValueInvokePos);
	validators.emplace(Opcode::VALUE_INVOKE_SIZE, &Validator::OpValueInvokeSize);
	validators.emplace(Opcode::VALUE_LIT_ARRAY, &Validator::OpValueLitArray);
	validators.emplace(Opcode::LIT_ARRAY_ELEM, &Validator::OpLitArrayElem);
	validators.emplace(Opcode::VALUE_LIT_BOOL, &Validator::OpValueLitBool);
	validators.emplace(Opcode::VALUE_LIT_FP, &Validator::OpValueLitFloat);
	validators.emplace(Opcode::VALUE_LIT_INT, &Validator::OpValueLitInt);
	validators.emplace(Opcode::VALUE_LIT_STR, &Validator::OpValueLitStr);
	validators.emplace(Opcode::VALUE_MEMBER, &Validator::OpValueMember);
	validators.emplace(Opcode::VALUE_NULL, &Validator::OpValueNull);
	validators.emplace(Opcode::VALUE_READ_VAR, &Validator::OpValueReadVar);
	validators.emplace(Opcode::VALUE_SIGN, &Validator::OpValueSign);
	validators.emplace(Opcode::VALUE_SUBARRAY, &Validator::OpValueSubarray);
	validators.emplace(Opcode::VALUE_UNSIGN, &Validator::OpValueUnsign);
	validators.emplace(Opcode::VALUE_VEC_SWIZZLE, &Validator::OpValueVecSwizzle);
	validators.emplace(Opcode::VALUE_ZERO, &Validator::OpValueZero);

	validators.emplace(Opcode::RETURN, &Validator::OpReturn);
	validators.emplace(Opcode::RETURN_VALUE, &Validator::OpReturnValue);
	validators.emplace(Opcode::DISCARD, &Validator::OpDiscard);

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
						auto e = errors->err(std::vector<std::string>{ "CLLR instruction", std::to_string(i.index), "cannot reference itself" }, i.debugTkn);
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
				auto const reason = (*this.*fn->second)(lvl, i, codeAsm);

				if (reason != ValidReason::VALID)
				{
					auto e = errors->err(VALIDATION_REASONS[(int)reason], i.debugTkn);

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

		if (i.debugTkn.exists())
		{
			std::vector<std::string> msg;

			msg.push_back("CLLR instruction");

			if (i.index == 0)
			{
				msg.push_back(std::string(OP_NAMES.at(i.op)));
			}
			else
			{
				msg.push_back(std::to_string(i.index));
			}

			msg.push_back("does not have a debug token");

			auto e = errors->err(msg);

			e->note(codeGenErr);
			e->note(i.toStr());

		}

	}

	return errors->empty();
}

CLLR_VALID_IMPL(OpNoop)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpShaderStage)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	//TODO validate shader stage

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpShaderStageEnd)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpFunction)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpVarFuncArg)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpFunctionEnd)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_REF(i.refs[0], Opcode::FUNCTION);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpVarLocal)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpVarGlobal)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpVarShaderIn)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpVarShaderOut)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpVarDescriptor)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpCall)
{
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

CLLR_VALID_IMPL(OpCallArg)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_REFS(2);
	CLLR_VALID_MAX_OPS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpTypeVoid)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_REFS;
	CLLR_VALID_NO_OPS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpTypeFloat)
{
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

CLLR_VALID_IMPL(OpTypeIntSign)
{
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

CLLR_VALID_IMPL(OpTypeIntUnsign)
{
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

CLLR_VALID_IMPL(OpTypeArray)
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

CLLR_VALID_IMPL(OpTypeVector)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);


	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpTypeMatrix)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	//TODO consider matrix semantics

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpTypeTexture)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_OP_NONZERO(i.operands[0]);

	//TODO consider matrix semantics

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpTypeStruct)
{
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

CLLR_VALID_IMPL(OpTypeBool)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpTypePtr)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_TYPE(i.refs[0]);

	return ValidReason::VALID;
}

//TOOD oops, I haven'typeHint begun to implement this yet
CLLR_VALID_IMPL(OpTypeTuple)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpStructMember)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_TYPE(i.refs[0]);
	CLLR_VALID_REF(i.refs[1], Opcode::TYPE_STRUCT);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpStructEnd)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_REF(i.refs[0], Opcode::TYPE_STRUCT);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpScopeBegin)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	scopes.push(i.index);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpScopeEnd)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_REF(i.refs[0], Opcode::SCOPE_BEGIN);

	if (scopes.top() != i.refs[0])
	{
		return ValidReason::INVALID_MISC;
	}

	scopes.pop();

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpLabel)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpJump)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_REF(i.refs[0], Opcode::LABEL);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpJumpCond)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_REF(i.refs[0], Opcode::LABEL);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpLoop)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	//TODO recall loop semantics
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpAssign)
{
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

CLLR_VALID_IMPL(OpCompare)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueCast)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_TYPE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueConstruct)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpConstructArg)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueDeref)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueExpand)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueExpr)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueExprUnary)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueIntToFP)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueInvokePos)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueInvokeSize)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueLitArray)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpLitArrayElem)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_REF(i.refs[0], Opcode::TYPE_ARRAY);
	CLLR_VALID_TYPE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueLitBool)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueLitFloat)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueLitInt)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(2);
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueLitStr)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_NO_REFS;
	//TODO string support

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueMember)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_MAX_OPS(1);
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueNull)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueReadVar)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VAR(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueSample)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueSign)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueSubarray)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(2);

	CLLR_VALID_VALUE(i.refs[0]);
	CLLR_VALID_VALUE(i.refs[1]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueUnsign)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpValueVecSwizzle)
{
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

CLLR_VALID_IMPL(OpValueZero)
{
	CLLR_VALID_HAS_ID;
	CLLR_VALID_HAS_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpReturn)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_NO_REFS;

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpReturnValue)
{
	CLLR_VALID_NO_ID;
	CLLR_VALID_NO_OUT;
	CLLR_VALID_NO_OPS;
	CLLR_VALID_MAX_REFS(1);

	CLLR_VALID_VALUE(i.refs[0]);

	return ValidReason::VALID;
}

CLLR_VALID_IMPL(OpDiscard)
{
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
