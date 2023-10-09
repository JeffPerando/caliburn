
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

bool cllr::TypeChecker::check(sptr<LowType> targetType, in<TypedSSA> rhs, out<TypedSSA> result, out<Assembler> codeAsm, Operator op) const
{
	result = rhs;

	while (true)
	{
		if (result.type->id == targetType->id)
		{
			return true;
		}

		cllr::SSA fnID = 0;
		
		switch (result.type->typeCheck(targetType, fnID, op))
		{
			case TypeCheckResult::COMPATIBLE: return true;
			case TypeCheckResult::INCOMPATIBLE: return false;
			case TypeCheckResult::WIDEN: {
				auto t = codeAsm.pushType(Instruction(result.type->category, { targetType->getBitWidth() }));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_EXPAND, {}, { result.value }, t->id));
				result = TypedSSA(t, v);
			}; break;
			case TypeCheckResult::BITCAST_TO_INT: {
				//TODO reconsider this conversion method
				//TODO get power of 2 for getBitWidth(). LowStruct can return a non-power-of-2.
				auto t = codeAsm.pushType(Instruction(Opcode::TYPE_INT_UNSIGN, { targetType->getBitWidth() }));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_CAST, {}, { result.value }, t->id));
				result = TypedSSA(t, v);
			}; break;
			case TypeCheckResult::INT_TO_FLOAT: {
				auto t = codeAsm.pushType(Instruction(Opcode::TYPE_FLOAT, { result.type->getBitWidth() }));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_INT_TO_FP, {}, { result.value }, t->id));
				result = TypedSSA(t, v);
			}; break;
			case TypeCheckResult::METHOD_CALL: {
				auto callID = codeAsm.pushNew(Instruction(Opcode::CALL, { 1 }, { fnID }, targetType->id));
				codeAsm.push(Instruction(Opcode::CALL_ARG, { 0 }, { result.value }));
				result = TypedSSA(targetType, callID);
			}; break;
			//this is to prevent an infinite loop
			default: return false;
		}

	}

}
