
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ValueStatement : public Statement
	{
		ValueStatement(uint32_t t) : Statement(t) {}
		~ValueStatement() { Statement::~Statement(); }

		virtual bool isLValue() = 0;

	};

	struct LiteralIntStmnt : public ValueStatement
	{
		int32_t value = 0;

		LiteralIntStmnt() : ValueStatement(0) {}

		bool isLValue()
		{
			return false;
		}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			if (!registered)
			{
				codeAsm->pushSSA(spirv::OpConstant(0));
				//codeAsm->push(codeAsm->pushType("int32"));

			}

			return ssa;
		}

	};

}
