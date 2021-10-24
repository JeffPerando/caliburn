
#pragma once

#include <map>
#include <string>

#include "langcore.h"

namespace caliburn
{
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
