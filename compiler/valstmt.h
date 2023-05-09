
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ValueStatement : public Statement
	{
		const uptr<Value> val;

		ValueStatement(uptr<Value> v) : Statement(StatementType::UNKNOWN), val(std::move(v)) {}

		virtual sptr<Token> firstTkn() const override
		{
			return val->firstTkn();
		}

		virtual sptr<Token> lastTkn() const override
		{
			return val->lastTkn();
		}

		virtual void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		virtual void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			val->resolveSymbols(table);

		}

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			val->emitValueCLLR(codeAsm);

		}

	};
}
