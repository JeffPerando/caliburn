
#include "cllr/cllrtype.h"

#include "cllr/cllrasm.h"
#include "cllr/cllrtype.h"

using namespace caliburn;

bool cllr::LowType::addConverter(SSA type, SSA fnID)
{
	return false;
}

cllr::SSA cllr::LowType::getConverter(SSA type)
{
	return 0;
}

bool cllr::LowType::addMember(SSA typeID, sptr<const LowType> typeImpl)
{
	return false;
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

	if (result == ConversionResult::NO_CONVERSION)
	{
		return true;
	}
	else if (result == ConversionResult::INCOMPATIBLE)
	{
		return false;
	}

	do
	{
		switch (result)
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
				auto fnID = rhsType->getConverter(targetType);
				auto callID = codeAsm.pushNew(Instruction(Opcode::CALL, { 1 }, { fnID }));
				codeAsm.push(Instruction(Opcode::CALL_ARG, { 0 }, { rhs.value }));
				rhs = TypedSSA(targetType, callID);
			}; break;
			case ConversionResult::INCOMPATIBLE: return false;
			default: break;
		}

		rhsType = codeAsm.getType(rhs.type);
		result = rhsType->isConvertibleTo(targetType, lhsType, op);

	} while (result != ConversionResult::NO_CONVERSION);

	return true;
}
