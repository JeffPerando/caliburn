
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ValueStatement : public Statement
	{
		ValueStatement(StatementType t) : Statement(t) {}
		virtual ~ValueStatement() {}

		virtual bool isLValue() = 0;

	};

	struct LiteralIntStmnt : public ValueStatement
	{
		int32_t value = 0;

		LiteralIntStmnt() : ValueStatement(StatementType::VALUE) {}

		bool isLValue()
		{
			return false;
		}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			
		}

	};

}
