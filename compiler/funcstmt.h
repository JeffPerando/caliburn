
#pragma once

#include "ast.h"

namespace caliburn
{
	struct FunctionStatement : public Statement
	{
		Token* first;
		Token* name;

		cllr::SSA funcID = 0;

		std::vector<Variable>* args;
		ScopeStatement* body = nullptr;
		
		FunctionStatement(Statement* parent) : Statement(StatementType::FUNCTION, parent) {}

		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			funcID = codeAsm.createSSA(cllr::Opcode::FUNCTION);

			body->getSSAs(codeAsm);

		}

		virtual void declSymbols(SymbolTable& table) override
		{
			table.add(name->str, SymbolType::FUNCTION, this);

			body->declSymbols(table);

		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			body->resolveSymbols(table);

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			codeAsm.push(funcID, cllr::Opcode::FUNCTION, {});

			body->emitDeclCLLR(codeAsm);

		}

	};

}
