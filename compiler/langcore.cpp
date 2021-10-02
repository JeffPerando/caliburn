
#include "statement.h"
#include "type.h"

using namespace caliburn;

StorageModifier caliburn::parseStorageMod(std::string str)
{
	if (str == "public")
		return StorageModifier::PUBLIC;
	if (str == "protected")
		return StorageModifier::PROTECTED;
	if (str == "private")
		return StorageModifier::PRIVATE;
	if (str == "shared")
		return StorageModifier::SHARED;
	if (str == "static")
		return StorageModifier::STATIC;

	return StorageModifier::NONE;
}

bool operator==(const FunctionArg& lhs, const FunctionArg& rhs)
{
	if (lhs.name != rhs.name)
	{
		return false;
	}

	if (*lhs.type != *rhs.type)
	{
		return false;
	}

	return true;
}

bool operator!=(const FunctionArg& lhs, FunctionArg& rhs)
{
	return !(lhs == rhs);
}

bool FunctionSignature::operator==(const FunctionSignature& rhs) const
{
	if (name != rhs.name)
	{
		return false;
	}

	if (memberType != rhs.memberType)
	{
		if (memberType == nullptr || rhs.memberType == nullptr)
		{
			return false;
		}

		if (*memberType != *rhs.memberType)
		{
			return false;
		}

	}

	if (*returnType != *rhs.returnType)
	{
		return false;
	}

	if (args.size() != rhs.args.size())
	{
		return false;
	}

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (*args[i] != *rhs.args[i])
		{
			return false;
		}

	}

	return true;
}

bool SymbolTable::add(Symbol* sym)
{
	if (sym->name.length() == 0 || symMap[sym->name])
	{
		return false;
	}

	symList.push_back(sym);
	symMap.emplace(sym->name, sym);
	return true;
}

Symbol* SymbolTable::resolve(std::string name)
{
	return symMap[name];
}

