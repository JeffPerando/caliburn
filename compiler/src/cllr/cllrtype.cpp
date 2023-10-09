
#include "cllr/cllrtype.h"

#include "cllr/cllrasm.h"

using namespace caliburn;

cllr::TypeCheckResult cllr::TypeChecker::lookup(sptr<LowType> targetType, in<TypedSSA> rhs, in<Assembler> codeAsm) const
{
	if (targetType->id == rhs.type->id)
	{
		return TypeCheckResult::COMPATIBLE;
	}

	cllr::SSA fnID = 0;
	return rhs.type->typeCheck(targetType, fnID);
}

bool cllr::TypeChecker::check(sptr<LowType> targetType, in<TypedSSA> rhs, out<TypedSSA> resultVal, out<Assembler> codeAsm, Operator op) const
{
	resultVal = rhs;

	if (targetType->id == rhs.type->id)
	{
		return true;
	}

	cllr::SSA fnID = 0;

	auto result = rhs.type->typeCheck(targetType, fnID, op);

	if (result == TypeCheckResult::COMPATIBLE)
	{
		return true;
	}
	else if (result == TypeCheckResult::INCOMPATIBLE)
	{
		return false;
	}

	do
	{
		switch (result)
		{
			case TypeCheckResult::WIDEN: {
				auto t = codeAsm.pushType(Instruction(resultVal.type->category, {targetType->getBitWidth()}, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_EXPAND, {}, { resultVal.value }, 0));
				resultVal = TypedSSA(t, v);
			}; break;
			case TypeCheckResult::BITCAST_TO_INT: {
				//TODO reconsider this conversion method
				//TODO get power of 2 for getBitWidth(). LowStruct can return a non-power-of-2.
				auto t = codeAsm.pushType(Instruction(Opcode::TYPE_INT_SIGN, { targetType->getBitWidth() }, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_CAST, {}, { resultVal.value }, t->id));
				resultVal = TypedSSA(t, v);
			}; break;
			case TypeCheckResult::INT_TO_FLOAT: {
				auto t = codeAsm.pushType(Instruction(Opcode::TYPE_FLOAT, { resultVal.type->getBitWidth() }, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_INT_TO_FP, {}, { resultVal.value }, t->id));
				resultVal = TypedSSA(t, v);
			}; break;
			case TypeCheckResult::METHOD_CALL: {
				auto callID = codeAsm.pushNew(Instruction(Opcode::CALL, { 1 }, { fnID }));
				codeAsm.push(Instruction(Opcode::CALL_ARG, { 0 }, { resultVal.value }));
				resultVal = TypedSSA(targetType, callID);
			}; break;
			case TypeCheckResult::INCOMPATIBLE: return false;
			default: break;
		}

		result = resultVal.type->typeCheck(targetType, fnID, op);

	} while (result != TypeCheckResult::COMPATIBLE);

	return true;
}
