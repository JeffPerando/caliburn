
#pragma once

#include "statement.h"
#include "valuestmnt.h"

namespace caliburn
{
	struct FuncArg
	{
		NamedSSA type, name;
		ValueStatement* argDefault = nullptr;
	};

	struct FuncStatement : public Statement
	{
		NamedSSA returnType;
		std::string name = "FORGOT TO NAME YOUR FUNCTION!";
		uint32_t ssa = 0;
		std::vector<FuncArg> args;
		uint32_t calls = 0;
		Statement* funcBody = nullptr;

		FuncStatement() : Statement(0) {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			ssa = codeAsm->newAssign();

			codeAsm->push(spirv::OpFunction());
			//TODO code
			codeAsm->push(spirv::OpFunctionEnd());
		}
	};

}