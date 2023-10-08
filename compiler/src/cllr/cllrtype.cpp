
#include "cllr/cllrtype.h"

#include "cllr/cllrasm.h"
#include "cllr/cllrtype.h"

using namespace caliburn;

cllr::ConvertResult cllr::TypeChecker::lookup(SSA targetType, in<TypedSSA> rhs, in<Assembler> codeAsm) const
{
	if (targetType == rhs.type)
	{
		return ConvertResult::LGTM;
	}

	auto lhsType = codeAsm.getType(targetType);
	auto rhsType = codeAsm.getType(rhs.type);

	return rhsType->isConvertibleTo(targetType, lhsType, Operator::UNKNOWN);
}

bool cllr::TypeChecker::check(SSA targetType, out<TypedSSA> rhs, out<Assembler> codeAsm, Operator op) const
{
	if (targetType == rhs.type)
	{
		return true;
	}

	auto lhsType = codeAsm.getType(targetType);
	auto rhsType = codeAsm.getType(rhs.type);

	auto result = rhsType->isConvertibleTo(targetType, lhsType, op);

	if (result == ConvertResult::LGTM)
	{
		return true;
	}
	else if (result == ConvertResult::INCOMPATIBLE)
	{
		return false;
	}

	do
	{
		switch (result)
		{
			case ConvertResult::WIDEN: {
				auto [t, tImpl] = codeAsm.pushType(Instruction(rhsType->category, {lhsType->getBitWidth()}, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_EXPAND, {}, { rhs.value }, 0));
				rhs = TypedSSA(t, v);
			}; break;
			case ConvertResult::BITCAST_TO_INT: {
				//TODO reconsider this conversion method
				//TODO get power of 2 for getBitWidth(). LowStruct can return a non-power-of-2.
				auto [t, tImpl] = codeAsm.pushType(Instruction(Opcode::TYPE_INT_SIGN, { lhsType->getBitWidth() }, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_CAST, {}, { rhs.value }, t));
				rhs = TypedSSA(t, v);
			}; break;
			case ConvertResult::INT_TO_FLOAT: {
				auto [t, tImpl] = codeAsm.pushType(Instruction(Opcode::TYPE_FLOAT, { rhsType->getBitWidth() }, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_INT_TO_FP, {}, { rhs.value }, t));
				rhs = TypedSSA(t, v);
			}; break;
			/*case ConvertResult::METHOD_CONVERSION: {
				auto fnID = rhsType->getConverter(targetType);
				auto callID = codeAsm.pushNew(Instruction(Opcode::CALL, { 1 }, { fnID }));
				codeAsm.push(Instruction(Opcode::CALL_ARG, { 0 }, { rhs.value }));
				rhs = TypedSSA(targetType, callID);
			}; break;*/
			case ConvertResult::INCOMPATIBLE: return false;
			default: break;
		}

		rhsType = codeAsm.getType(rhs.type);
		result = rhsType->isConvertibleTo(targetType, lhsType, op);

	} while (result != ConvertResult::LGTM);

	return true;
}
