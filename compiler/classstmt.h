
#pragma once

#include "funcstmt.h"
#include "structstmt.h"

namespace caliburn
{
	struct ClassStatement : public StructStatement
	{
		ClassStatement(Module* p) : StructStatement(StatementType::CLASS, p) {}
		virtual ~ClassStatement() {}

	};

	struct MemberFunctionStatement : public FunctionStatement
	{
		MemberFunctionStatement(Module* parent) : FunctionStatement(parent) {}
		virtual ~MemberFunctionStatement() {}

	};


}