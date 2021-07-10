
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ValueStatement : public Statement
	{
	public:
		ValueStatement(uint32_t t) : Statement(t) {}
		~ValueStatement() { Statement::~Statement(); }

	};

	struct LiteralIntStmnt : public ValueStatement
	{
	public:
		int32_t value = 0;

		LiteralIntStmnt() : ValueStatement(0) {}

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
