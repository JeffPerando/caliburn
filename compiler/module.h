
#pragma once

#include <map>
#include <string>

#include "statement.h"

namespace caliburn
{
	//TODO Eventually make a virtual with both precompiled and parsed code variants
	//It'll mean potentially doubling up on Statement supers, but ehh...
	class Module
	{
		std::map<std::string, Statement*> declarations;
	
	public:
		void putDecl(std::string name, Statement* stmt)
		{
			declarations.emplace(name, stmt);

		}

		Statement* resolve(std::string name)
		{
			return declarations[name];
		}

	};

}
