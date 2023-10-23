
#include "cllr/cllrtypes.h"

using namespace caliburn::cllr;

TypeCheckResult LowFloat::typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op) const
{
	if (op != Operator::NONE)
	{
		if (op == Operator::APPEND || op == Operator::INTDIV)
		{
			return TypeCheckResult::INCOMPATIBLE;
		}

		auto opCat = OP_CATEGORIES.at(op);

		if (opCat == OpCategory::BITWISE)
		{
			return TypeCheckResult::INCOMPATIBLE;
		}

	}

	if (target->category != Opcode::TYPE_FLOAT)
	{
		return TypeCheckResult::INCOMPATIBLE;
	}

	if (width > target->getBitWidth())
	{
		return TypeCheckResult::INCOMPATIBLE;
	}

	if (target->getBitWidth() > width)
	{
		return TypeCheckResult::WIDEN;
	}

	return TypeCheckResult::COMPATIBLE;
}

TypeCheckResult LowInt::typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op) const
{
	if (op == Operator::APPEND)
	{
		return TypeCheckResult::INCOMPATIBLE;
	}

	bool sign = (category == Opcode::TYPE_INT_SIGN);

	if (!sign && (op == Operator::NEG || op == Operator::ABS))
	{
		return TypeCheckResult::INCOMPATIBLE;
	}
	
	if (target->category == Opcode::TYPE_FLOAT || op == Operator::DIV)
	{
		if (width > caliburn::MAX_FLOAT_BITS)
		{
			return TypeCheckResult::INCOMPATIBLE;
		}

		return TypeCheckResult::INT_TO_FLOAT;
	}

	if (target->category != category)
	{
		return TypeCheckResult::INCOMPATIBLE;
	}

	if (target->getBitWidth() > width)
	{
		return TypeCheckResult::WIDEN;
	}

	if (width > target->getBitWidth())
	{
		return TypeCheckResult::INCOMPATIBLE;
	}

	return TypeCheckResult::COMPATIBLE;
}

TypeCheckResult LowBool::typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op) const
{
	auto opCat = OP_CATEGORIES.at(op);

	if (opCat == OpCategory::LOGICAL)
	{
		return TypeCheckResult::COMPATIBLE;
	}

	if (target->category == Opcode::TYPE_INT_SIGN || target->category == Opcode::TYPE_INT_UNSIGN)
	{
		return TypeCheckResult::BITCAST_TO_INT;
	}

	return TypeCheckResult::INCOMPATIBLE;
}

TypeCheckResult LowStruct::typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op) const
{
	//TODO operator overloads
	if (auto convert = conversions.find(target->id); convert != conversions.end())
	{
		fnID = convert->second;
		return TypeCheckResult::METHOD_CALL;
	}

	return TypeCheckResult::INCOMPATIBLE;
}

//TODO finish implementing

bool LowStruct::addMember(std::string name, sptr<const LowType> typeImpl)
{
	return false;
}

LowMember LowStruct::getMember(std::string name, out<cllr::Assembler> codeAsm) const
{
	return LowMember();
}

std::vector<std::string> LowStruct::getMembers() const
{
	return std::vector<std::string>();
}

bool LowStruct::setMemberFns(std::string name, sptr<FunctionGroup> fn)
{
	return false;
}

sptr<caliburn::Function> LowStruct::getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const
{
	return nullptr;
}
