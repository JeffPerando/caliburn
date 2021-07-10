
#pragma once

#include "scopestmnt.h"

namespace caliburn
{
	struct ImportStmnt : public Statement
	{
		std::string imported;

		ImportStmnt() : Statement(0) {}

	};

	struct ShaderStmnt : public Statement
	{
		ScopeStatement* inner = nullptr;
		std::string name;
		std::vector<std::string> supers, descriptors;

		ShaderStmnt() : Statement(0) {}
	};

}
