
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

	if (memberOf != rhs.memberOf)
	{
		if (memberOf == nullptr || rhs.memberOf == nullptr)
		{
			return false;
		}

		if (*memberOf != *rhs.memberOf)
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
	if (sym->name.length() == 0 || symMap.count(sym->name) != 0)
	{
		return false;
	}

	symList.push_back(sym);
	symMap.emplace(sym->name, sym);
	return true;
}

bool SymbolTable::alias(std::string name, Symbol* sym)
{
	if (name.length() == 0 || sym == nullptr)
	{
		return false;
	}

	//ensure the original name DOES exist, and the new name doesn't
	if (symMap.count(sym->name) == 0 || symMap.count(name) != 0)
	{
		return false;
	}

	//DO NOT ADD THE SYMBOL TO THE SYMBOL LIST; IT'S USED FOR DELETES
	//ADDING WILL RESULT IN A DOUBLE DELETE
	symMap.emplace(name, sym);

	return true;
}

Symbol* SymbolTable::resolve(std::string name)
{
	auto result = symMap.find(name);

	if (result != symMap.end())
	{
		return result->second;
	}

	if (parent)
	{
		return parent->resolve(name);
	}
	
	return nullptr;
}

