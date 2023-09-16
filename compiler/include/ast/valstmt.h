
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ValueStatement : Statement
	{
		const sptr<Value> val;

		ValueStatement(sptr<Value> v) : Statement(StatementType::UNKNOWN), val(v) {}

		sptr<Token> firstTkn() const override
		{
			return val->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return val->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			return val->emitValueCLLR(table, codeAsm).value;
		}

	};
}
