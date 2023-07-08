
#pragma once

#include "ast.h"

namespace caliburn
{
	struct SetterStatement : public Statement
	{
		sptr<Value> lValue = nullptr;
		sptr<Value> rValue = nullptr;

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

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			auto lhs = lValue->emitValueCLLR(table, codeAsm);
			auto rhs = rValue->emitValueCLLR(table, codeAsm);

			codeAsm.push(0, cllr::Opcode::ASSIGN, {}, {lhs, rhs});

		}

	};

}