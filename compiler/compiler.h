
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

		int32_t compile(std::string file, std::vector<std::string>& compiled)
		{
			std::string src;
			std::vector<Token> tokens;
			std::vector<Statement*> ast;

			caliburn::tokenize(src, tokens);

			Parser parser;
			parser.parse(&tokens, &ast);

			return -1;
		}

	};

}
