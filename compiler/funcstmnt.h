
#pragma once

#include "statement.h"
#include "valuestmnt.h"

namespace caliburn
{
	enum class FuncControl : uint32_t
	{
		None = 0x00,
		Inline = 0x01,
		DontLine = 0x02,
		Pure = 0x04,
		Const = 0x08

	};

	struct FuncArg
	{
		ParsedType* type = nullptr;
		std::string name;
		ValueStatement* argDefault = nullptr;
		uint32_t ssa = 0;

	};

	struct FunctionStatement : public Statement
	{
		ParsedType* returnType = nullptr;
		std::string name = "FORGOT TO NAME YOUR FUNCTION!";
		std::vector<FuncArg> args;
		FuncControl control = FuncControl::Inline;
		uint32_t calls = 0;
		Statement* funcBody = nullptr;

		FunctionStatement() : Statement(0) {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			uint32_t ssa = codeAsm->newAssign();

			uint32_t returnTypeSSA = codeAsm->pushType(returnType);

			codeAsm->push(spirv::OpFunction());
			codeAsm->push(returnTypeSSA); //TODO proper type resolving
			codeAsm->push(ssa);
			codeAsm->push((uint32_t)control);
			codeAsm->push(returnTypeSSA);

			for (auto arg : args)
			{
				arg.ssa = codeAsm->newAssign();
				codeAsm->pushAll({ spirv::OpFunctionParameter(), codeAsm->pushType(arg.type), arg.ssa });

			}

			funcBody->toSPIRV(codeAsm);

			codeAsm->push(spirv::OpFunctionEnd());
			
			return ssa;
		}

	};

}