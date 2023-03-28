
#pragma once

#include "ast.h"

namespace caliburn
{
	struct SetterStatement : public Statement
	{
		Value* lhs = nullptr;
		Value* rhs = nullptr;

		SetterStatement(Statement* p) : Statement(StatementType::SETTER, p) {}

		Token* firstTkn() const override
		{
			return lhs->firstTkn();
		}

		Token* lastTkn() const override
		{
			return rhs->lastTkn();
		}

		void declSymbols(SymbolTable& table) override {}

		void resolveSymbols(const SymbolTable& table) override
		{
			lhs->resolveSymbols(table);
			rhs->resolveSymbols(table);

		}

		void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			auto id = rhs->emitLoadCLLR(codeAsm);
			lhs->emitStoreCLLR(codeAsm, id);

		}

	};

}