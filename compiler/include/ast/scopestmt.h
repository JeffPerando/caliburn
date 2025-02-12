
#pragma once

#include "ast.h"

namespace caliburn
{
	/*
	Defines a scope, which contains its own symbol table. Said table can shadow other symbols.
	*/
	struct ScopeStmt : Expr
	{
		Token first;
		Token last;

		std::vector<sptr<Expr>> stmts;

		sptr<SymbolTable> scopeTable = new_sptr<SymbolTable>();

		ScopeStmt(ExprType exType = ExprType::SCOPE) : Expr(exType) {}

		virtual ~ScopeStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return last;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		virtual ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct BreakStmt : Expr
	{
		const Token tkn;

		BreakStmt(in<Token> t) : Expr(ExprType::UNKNOWN), tkn(t) {}

		virtual ~BreakStmt() = default;

		Token firstTkn() const noexcept override
		{
			return tkn;
		}

		Token lastTkn() const noexcept override
		{
			return tkn;
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			ss << "break";
		}

		virtual ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct ContinueStmt : Expr
	{
		const Token tkn;

		ContinueStmt(in<Token> t) : Expr(ExprType::UNKNOWN), tkn(t) {}

		virtual ~ContinueStmt() = default;

		Token firstTkn() const noexcept override
		{
			return tkn;
		}

		Token lastTkn() const noexcept override
		{
			return tkn;
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			ss << "continue";
		}

		virtual ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct DiscardStmt : Expr
	{
		const Token tkn;

		DiscardStmt(in<Token> t) : Expr(ExprType::UNKNOWN), tkn(t) {}

		virtual ~DiscardStmt() = default;

		Token firstTkn() const noexcept override
		{
			return tkn;
		}

		Token lastTkn() const noexcept override
		{
			return tkn;
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			ss << "discard";
		}

		virtual ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct ReturnStmt : Expr
	{
		const Token first;

		sptr<Expr> retValue = nullptr;

		ReturnStmt(in<Token> t) : Expr(ExprType::UNKNOWN), first(t) {}

		virtual ~ReturnStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			if (retValue != nullptr)
			{
				return retValue->lastTkn();
			}

			return first;
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			ss << "return";

			if (retValue != nullptr)
			{
				ss << ' ';
				retValue->prettyPrint(ss);
			}

		}

		virtual ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct PassStmt : Expr
	{
		const Token tkn;

		PassStmt(in<Token> t) : Expr(ExprType::UNKNOWN), tkn(t) {}

		virtual ~PassStmt() = default;

		Token firstTkn() const noexcept override
		{
			return tkn;
		}

		Token lastTkn() const noexcept override
		{
			return tkn;
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			ss << "pass";
		}

		virtual ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct UnreachableStmt : Expr
	{
		const Token tkn;

		UnreachableStmt(in<Token> t) : Expr(ExprType::UNKNOWN), tkn(t) {}

		virtual ~UnreachableStmt() = default;

		Token firstTkn() const noexcept override
		{
			return tkn;
		}

		Token lastTkn() const noexcept override
		{
			return tkn;
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			ss << "unreachable";
		}

		virtual ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

}
