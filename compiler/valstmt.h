
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ValueStatement : public Statement
	{
		const ptr<Value> val;

		ValueStatement(ptr<Value> v) : Statement(StatementType::UNKNOWN), val(v) {}

		virtual Token* firstTkn() const override
		{
			return val->firstTkn();
		}

		virtual Token* lastTkn() const override
		{
			return val->lastTkn();
		}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override {}

		virtual void resolveSymbols(ref<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			val->resolveSymbols(table);

		}

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			val->emitValueCLLR(codeAsm);

		}

	};
}
