
#pragma once

#include "ast.h"

namespace caliburn
{
	/*
	Defines a scope, which contains its own symbol table. Said table can shadow other symbols.
	*/
	struct ScopeStmt : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<Statement>> stmts;

		sptr<SymbolTable> scopeTable = nullptr;

		ScopeStmt(StmtType stmtType = StmtType::SCOPE) : Statement(stmtType) {}
		virtual ~ScopeStmt() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override;

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct BreakStmt : Statement
	{
		const sptr<Token> tkn;

		BreakStmt(sptr<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}
		virtual ~BreakStmt() {}

		sptr<Token> firstTkn() const override
		{
			return tkn;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> tkn;

		ContinueStmt(sptr<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}
		virtual ~ContinueStmt() {}

		sptr<Token> firstTkn() const override
		{
			return tkn;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> tkn;

		DiscardStmt(sptr<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}
		virtual ~DiscardStmt() {}

		sptr<Token> firstTkn() const override
		{
			return tkn;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> first;

		sptr<Value> retValue = nullptr;

		ReturnStmt(sptr<Token> t) : Statement(StmtType::UNKNOWN), first(t) {}
		virtual ~ReturnStmt() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> tkn;

		PassStmt(sptr<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}
		virtual ~PassStmt() {}

		sptr<Token> firstTkn() const override
		{
			return tkn;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> tkn;

		UnreachableStmt(sptr<Token> t) : Statement(StmtType::UNKNOWN), tkn(t) {}
		virtual ~UnreachableStmt() {}

		sptr<Token> firstTkn() const override
		{
			return tkn;
		}

		sptr<Token> lastTkn() const override
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
