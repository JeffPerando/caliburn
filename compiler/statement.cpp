
#include "statement.h"

using namespace caliburn;

Visibility caliburn::strToVis(std::string str)
{
	if (str == "public")
		return Visibility::PUBLIC;
	if (str == "protected")
		return Visibility::PROTECTED;
	if (str == "private")
		return Visibility::PRIVATE;
	if (str == "shared")
		return Visibility::SHARED;
	if (str == "static")
		return Visibility::STATIC;

	return Visibility::NONE;
}
