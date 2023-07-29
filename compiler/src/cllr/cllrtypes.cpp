
#include "cllr/cllrtypes.h"

using namespace caliburn::cllr;

ConversionResult LowFloat::isConvertibleTo(SSA other, sptr<const LowType> otherImpl) const
{
	if (otherImpl->category != Opcode::TYPE_FLOAT)
	{
		return ConversionResult::INCOMPATIBLE;
	}

	if (otherImpl->getBitWidth() > width)
	{
		return ConversionResult::WIDEN;
	}

	return ConversionResult::NO_CONVERSION;
}

ConversionResult LowInt::isConvertibleTo(SSA other, sptr<const LowType> otherImpl) const
{
	if (otherImpl->category == category)
	{
		if (otherImpl->getBitWidth() > width)
		{
			return ConversionResult::WIDEN;
		}

		return ConversionResult::NO_CONVERSION;
	}

	if (otherImpl->category == Opcode::TYPE_FLOAT)
	{
		if (width <= caliburn::MAX_FLOAT_BITS)
		{
			return ConversionResult::INT_TO_FLOAT;
		}

	}

	return ConversionResult::INCOMPATIBLE;
}

ConversionResult LowStruct::isConvertibleTo(SSA other, sptr<const LowType> otherImpl) const
{
	if (conversions.find(other) != conversions.end())
	{
		return ConversionResult::METHOD_CONVERSION;
	}

	return ConversionResult::INCOMPATIBLE;
}

ConversionResult LowBool::isConvertibleTo(SSA other, sptr<const LowType> otherImpl) const
{
	if (otherImpl->category == Opcode::TYPE_INT_SIGN || otherImpl->category == Opcode::TYPE_INT_UNSIGN)
	{
		return ConversionResult::BITCAST_TO_INT;
	}

	return ConversionResult::INCOMPATIBLE;
}
