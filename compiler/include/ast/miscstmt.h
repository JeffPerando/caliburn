
#pragma once

#include "ast.h"

namespace caliburn
{
	struct SetterStatement : public Statement
	{
		uptr<Value> lValue = nullptr;
		uptr<Value> rValue = nullptr;

		SetterStatement() : Statement(StatementType::SETTER) {}

		sptr<Token> firstTkn() const override
		{
			return lValue->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return rValue->lastTkn();
		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			lValue->resolveSymbols(table);
			rValue->resolveSymbols(table);

		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			auto lhs = lValue->emitValueCLLR(codeAsm);
			auto rhs = rValue->emitValueCLLR(codeAsm);

			codeAsm.push(0, cllr::Opcode::ASSIGN, {}, {lhs, rhs});

		}

	};

}