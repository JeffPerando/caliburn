
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ImportStmt : Expr
	{
		const Token first;
		const Token name;
		Token alias;

		ImportStmt(in<Token> f, in<Token> n) :
			Expr(ExprType::IMPORT), first(f), name(n) {}

		ImportStmt(in<Token> f, in<Token> n, in<Token> a) :
			Expr(ExprType::IMPORT), first(f), name(n), alias(a) {}

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

		ValueResult emitCodeCLLR(sptr<SymbolTable>, out<cllr::Assembler> codeAsm) const override
		{
			return ValueResult();
		}

	};

	struct ModuleStmt : Expr
	{
		const Token first, name;

		ModuleStmt(in<Token> s, in<Token> n) : Expr(ExprType::MODULE), first(s), name(n) {}
		
		virtual ~ModuleStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return name;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable>, out<cllr::Assembler> codeAsm) const override
		{
			return ValueResult();
		}

	};

}
