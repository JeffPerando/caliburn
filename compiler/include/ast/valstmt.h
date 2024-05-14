
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ValueStmt : Statement
	{
		const sptr<Value> val;

		ValueStmt(sptr<Value> v) : Statement(StmtType::UNKNOWN), val(v) {}

		Token firstTkn() const noexcept override
		{
			return val->firstTkn();
		}

		Token lastTkn() const noexcept override
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
