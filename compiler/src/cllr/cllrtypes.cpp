
#include "cllr/cllrtypes.h"

using namespace caliburn::cllr;

ConvertResult LowFloat::isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const
{
	if (otherImpl->category != Opcode::TYPE_FLOAT || op == Operator::INTDIV)
	{
		return ConvertResult::INCOMPATIBLE;
	}

	if (otherImpl->getBitWidth() > width)
	{
		return ConvertResult::WIDEN;
	}

	return ConvertResult::LGTM;
}

OpResult LowFloat::getOp(Operator op, out<cllr::SSA> fnID) const
{
	return OpResult();
}

ConvertResult LowInt::isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const
{
	if (otherImpl->category == category)
	{
		if (otherImpl->getBitWidth() > width)
		{
			return ConvertResult::WIDEN;
		}

		return ConvertResult::LGTM;
	}

	if (otherImpl->category == Opcode::TYPE_FLOAT || op == Operator::DIV)
	{
		if (width <= caliburn::MAX_FLOAT_BITS)
		{
			return ConvertResult::INT_TO_FLOAT;
		}

	}

	return ConvertResult::INCOMPATIBLE;
}

OpResult LowInt::getOp(Operator op, out<cllr::SSA> fnID) const
{
	return OpResult();
}

ConvertResult LowBool::isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const
{
	if (otherImpl->category == category)
	{
		return ConvertResult::LGTM;
	}

	if (otherImpl->category == Opcode::TYPE_INT_SIGN || otherImpl->category == Opcode::TYPE_INT_UNSIGN)
	{
		return ConvertResult::BITCAST_TO_INT;
	}

	return ConvertResult::INCOMPATIBLE;
}

OpResult LowBool::getOp(Operator op, out<cllr::SSA> fnID) const
{
	return OpResult::INCOMPATIBLE;
}

ConvertResult LowStruct::isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const
{
	if (conversions.find(other) != conversions.end())
	{
		return ConvertResult::METHOD_CONVERSION;
	}

	return ConvertResult::INCOMPATIBLE;
}

bool LowStruct::addMember(std::string name, SSA typeID, sptr<const LowType> typeImpl)
{
	return false;
}

bool LowStruct::getMember(std::string name, out<std::pair<uint32_t, sptr<LowType>>> member) const
{
	return false;
}

bool LowStruct::setMemberFns(std::string name, ref<sptr<FunctionGroup>> fn)
{
	return false;
}

sptr<caliburn::Function> LowStruct::getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const
{
	return sptr<Function>();
}
