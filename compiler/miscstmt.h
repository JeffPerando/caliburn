
#pragma once

#include "ast.h"

namespace caliburn
{
	struct SetterStatement : public Statement
	{
		Value* lValue = nullptr;
		Value* rValue = nullptr;

		SetterStatement() : Statement(StatementType::SETTER) {}

		Token* firstTkn() const override
		{
			return lValue->firstTkn();
		}

		Token* lastTkn() const override
		{
			return rValue->lastTkn();
		}

		void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override
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