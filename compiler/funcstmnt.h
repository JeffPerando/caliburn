
#pragma once

#include "statement.h"
#include "valuestmnt.h"

namespace caliburn
{
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
		spirv::FuncControl control = spirv::FuncControl::None;
		uint32_t calls = 0;
		Statement* funcBody = nullptr;

		FunctionStatement() : Statement(StatementType::FUNCTION) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm)
		{
			if (returnType == nullptr)
			{
				//TODO complain
			}

			//SSAs
			uint32_t returnTypeSSA = codeAsm->resolveType(returnType)->typeDeclSpirV(codeAsm);
			std::vector<uint32_t> argTypeSSAs(args.size());

			for (auto arg : args)
			{
				argTypeSSAs.push_back(codeAsm->resolveType(arg.type)->typeDeclSpirV(codeAsm));

			}

			uint32_t ssa = codeAsm->newAssign();
			uint32_t startSSA = codeAsm->newAssign();

			//push ops

			codeAsm->pushAll({ spirv::OpFunction(), returnTypeSSA, ssa, (uint32_t)control, returnTypeSSA});
			
			codeAsm->startScope();
			codeAsm->getCurrentScope()->label = startSSA;

			for (size_t i = 0; i < args.size(); ++i)
			{
				FuncArg* arg = &args[i];
				arg->ssa = codeAsm->newAssign();
				//TODO fix. parameters need to be in OpTypePointer Function form.
				//proper implementation will involve making either a TypeFunctionArg,
				//or a SPIR-V assembly helper that helps prevent redundancy
				codeAsm->pushAll({ spirv::OpFunctionParameter(), argTypeSSAs[i], arg->ssa });
				
			}

			codeAsm->pushAll({spirv::OpLabel(), startSSA});

			funcBody->SPIRVEmit(codeAsm);

			codeAsm->endScope();

			codeAsm->push(spirv::OpFunctionEnd());
			
			return ssa;
		}

	};

}