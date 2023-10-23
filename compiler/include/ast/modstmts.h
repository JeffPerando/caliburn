
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ImportStatement : Statement
	{
		const sptr<Token> first;
		sptr<Token> name = nullptr;
		sptr<Token> alias = nullptr;

		ImportStatement(sptr<Token> f) : Statement(StatementType::IMPORT), first(f) {}
		virtual ~ImportStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			if (alias == nullptr)
			{
				return name;
			}

			return alias;
		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

	struct ModuleStatement : Statement
	{
		const sptr<Token> first, name;

		ModuleStatement(sptr<Token> s, sptr<Token> n) : Statement(StatementType::MODULE), first(s), name(n) {}
		virtual ~ModuleStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return name;
		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

}
