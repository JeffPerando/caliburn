
#pragma once

#include "ast.h"

namespace caliburn
{
	struct SetterStatement : Statement
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

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			auto lhs = lValue->emitValueCLLR(table, codeAsm);
			auto rhs = rValue->emitValueCLLR(table, codeAsm);

			codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { lhs.value, rhs.value }));

		}

	};

}