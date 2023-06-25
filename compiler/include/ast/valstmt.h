
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ValueStatement : public Statement
	{
		const uptr<Value> val;

		ValueStatement(uptr<Value> v) : Statement(StatementType::UNKNOWN), val(std::move(v)) {}

		sptr<Token> firstTkn() const override
		{
			return val->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return val->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			val->resolveSymbols(table);

		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			val->emitValueCLLR(codeAsm);

		}

	};
}
