
#pragma once

#include "ast.h"

namespace caliburn
{
	/*
	Defines a scope, which contains its own symbol table. Said table can shadow other symbols.
	*/
	struct ScopeStmt : Statement
	{
		Token first;
		Token last;

		std::vector<uptr<Statement>> stmts;

		sptr<SymbolTable> scopeTable = nullptr;

		ScopeStmt(StmtType stmtType = StmtType::SCOPE) : Statement(stmtType) {}

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override;

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct BreakStmt : Statement
	{
		const Token tkn;

		BreakStmt(in<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct ContinueStmt : Statement
	{
		const Token tkn;

		ContinueStmt(in<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct DiscardStmt : Statement
	{
		const Token tkn;

		DiscardStmt(in<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct ReturnStmt : Statement
	{
		const Token first;

		sptr<Value> retValue = nullptr;

		ReturnStmt(in<Token> t) : Statement(StmtType::UNKNOWN), first(t) {}

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct PassStmt : Statement
	{
		const Token tkn;

		PassStmt(in<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct UnreachableStmt : Statement
	{
		const Token tkn;

		UnreachableStmt(in<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

}
