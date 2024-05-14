
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

LowMember LowVector::getMember(SSA objID, std::string_view name, out<cllr::Assembler> codeAsm) const
{
	static const HashMap<char, uint32_t> comps = {
		{'x', 0}, {'y', 1}, {'z', 2}, {'w', 3},
		{'r', 0}, {'g', 1}, {'b', 2}, {'a', 3}
	};
	
	if (name.length() > 4)
	{
		//TODO complain
		return LowMember();
	}

	if (name.length() == 1)
	{
		if (auto comp = comps.find(name[0]); comp != comps.end())
		{
			//return a normal member
			return std::pair(comp->second, innerType);
		}

		//TODO complain
		return LowMember();
	}

	OpArray swizzleComponents = {};

	for (size_t i = 0; i < name.length(); ++i)
	{
		if (auto comp = comps.find(name[i]); comp != comps.end())
		{
			swizzleComponents[i] = comp->second;
		}
		else
		{
			//TODO complain
			return LowMember();
		}

	}

	auto vecType = codeAsm.pushType(Instruction(Opcode::TYPE_VECTOR, { (uint32_t)name.length() }, { innerType->id }));
	auto swizzleVal = codeAsm.pushNew(Instruction(Opcode::VALUE_VEC_SWIZZLE, swizzleComponents, { objID }, vecType->id));

	return TypedSSA(vecType, swizzleVal);
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

bool LowStruct::addMember(std::string_view name, sptr<const LowType> typeImpl)
{
	return false;
}

LowMember LowStruct::getMember(SSA objID, std::string_view name, out<cllr::Assembler> codeAsm) const
{
	return LowMember();
}

std::vector<std::string> LowStruct::getMembers() const
{
	return std::vector<std::string>();
}

bool LowStruct::addMemberFn(sptr<Method> fn)
{
	return false;
}

sptr<caliburn::FunctionGroup> LowStruct::getMemberFns(std::string_view name) const
{
	return nullptr;
}
