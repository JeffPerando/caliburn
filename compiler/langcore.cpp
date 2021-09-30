
#include "statement.h"
#include "type.h"

caliburn::StorageModifier caliburn::parseStorageMod(std::string str)
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

bool caliburn::FunctionSignature::operator==(const FunctionSignature& rhs) const
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
