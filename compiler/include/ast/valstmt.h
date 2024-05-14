
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ValueStmt : Statement
	{
		const sptr<Value> val;

		ValueStmt(sptr<Value> v) : Statement(StmtType::UNKNOWN), val(v) {}

		sptr<Token> firstTkn() const override
		{
			return val->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return val->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			val->emitValueCLLR(table, codeAsm);
		}

	};

}
