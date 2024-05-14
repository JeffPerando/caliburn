
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ImportStmt : Statement
	{
		const Token first;
		const Token name;
		Token alias;

		ImportStmt(in<Token> f, in<Token> n) :
			Statement(StmtType::IMPORT), first(f), name(n) {}

		ImportStmt(in<Token> f, in<Token> n, in<Token> a) :
			Statement(StmtType::IMPORT), first(f), name(n), alias(a) {}

		virtual ~ImportStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			if (alias.exists())
			{
				return alias;
			}

			return name;
		}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

	struct ModuleStmt : Statement
	{
		const Token first, name;

		ModuleStmt(in<Token> s, in<Token> n) : Statement(StmtType::MODULE), first(s), name(n) {}
		
		virtual ~ModuleStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return name;
		}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

}
