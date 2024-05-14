
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ImportStmt : Statement
	{
		const sptr<Token> first;
		sptr<Token> name = nullptr;
		sptr<Token> alias = nullptr;

		ImportStmt(sptr<Token> f) : Statement(StmtType::IMPORT), first(f) {}
		virtual ~ImportStmt() {}

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

	struct ModuleStmt : Statement
	{
		const sptr<Token> first, name;

		ModuleStmt(sptr<Token> s, sptr<Token> n) : Statement(StmtType::MODULE), first(s), name(n) {}
		virtual ~ModuleStmt() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return name;
		}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

}
