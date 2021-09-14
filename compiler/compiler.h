
#pragma once

#include <string>
#include <vector>

#include "modlib.h"
#include "parser.h"

namespace caliburn
{
	class Compiler
	{
		ModuleLibrary* const modLib;

		Compiler(ModuleLibrary* m) : modLib(m) {}
		~Compiler() {}

		int32_t compile(std::string src, std::vector<std::pair<size_t, uint32_t*>>& compiled)
		{
			std::vector<Token> tokens;
			caliburn::tokenize(src, tokens);

			Parser parser;
			std::vector<Statement*> ast;
			parser.parse(&tokens, &ast);

			return -1;
		}

	};

}
