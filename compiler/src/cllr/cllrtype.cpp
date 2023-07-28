
#include "cllr/cllrasm.h"
#include "cllr/cllrtype.h"

using namespace caliburn;

bool cllr::LowType::addConverter(SSA type, SSA fnID)
{
	return false;
}

bool cllr::LowType::addMember(SSA typeID, sptr<const LowType> typeImpl)
{
	return false;
}

bool cllr::TypeChecker::check(ref<TypedSSA> lhs, ref<TypedSSA> rhs, Operator op, ref<Assembler> codeAsm) const
{
	if (lhs.type == rhs.type)
	{
		return true;
	}

	auto lhsType = codeAsm.getType(lhs.type);
	auto rhsType = codeAsm.getType(rhs.type);

	auto lhsRes = lhsType->isConvertibleTo(rhs.type, rhsType);
	auto rhsRes = rhsType->isConvertibleTo(lhs.type, lhsType);

	//TODO reconsider
	if (lhsRes == ConversionResult::NO_CONVERSION || rhsRes == ConversionResult::NO_CONVERSION)
	{
		return true;
	}

	if (lhsRes == rhsRes && rhsRes == ConversionResult::INCOMPATIBLE)
	{
		return false;
	}

	//TODO do LHS as well
	do
	{
		switch (rhsRes)
		{
			case ConversionResult::WIDEN: {
				auto t = codeAsm.pushType(Instruction(rhsType->category, { lhsType->getBitWidth() }, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_EXPAND, {}, { rhs.value }, 0));
				rhs = TypedSSA(t, v);
			}; break;
			case ConversionResult::BITCAST_TO_INT: {
				//TODO reconsider this conversion method
				//TODO get power of 2 for getBitWidth(). LowStruct can return a non-power-of-2.
				auto t = codeAsm.pushType(Instruction(Opcode::TYPE_INT_SIGN, { lhsType->getBitWidth() }, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_CAST, {}, { rhs.value }, t));
				rhs = TypedSSA(t, v);
			}; break;
			case ConversionResult::INT_TO_FLOAT: {
				auto t = codeAsm.pushType(Instruction(Opcode::TYPE_FLOAT, { rhsType->getBitWidth() }, {}));
				auto v = codeAsm.pushNew(Instruction(Opcode::VALUE_INT_TO_FP, {}, { rhs.value }, t));
				rhs = TypedSSA(t, v);
			}; break;
			case ConversionResult::METHOD_CONVERSION: {
				//TODO finish

				SSA fnID = 0;
				auto callID = codeAsm.pushNew(Instruction(Opcode::CALL, { 1 }, { fnID }));
				codeAsm.push(Instruction(Opcode::CALL_ARG, { 0 }, { lhs.value }));
				lhs = TypedSSA(rhs.type, callID);

			}; break;
			case ConversionResult::INCOMPATIBLE: return false;
			default: break;
		}

		rhsType = codeAsm.getType(rhs.type);
		rhsRes = rhsType->isConvertibleTo(lhs.type, lhsType);

	} while (rhsRes != ConversionResult::NO_CONVERSION);

	return true;
}
