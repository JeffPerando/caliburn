
#include "statement.h"

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
