
#pragma once

#include "funcstmt.h"
#include "structstmt.h"

namespace caliburn
{
	struct ClassStatement : public StructStatement
	{
		ClassStatement(sptr<Token> f) : StructStatement(f, StatementType::CLASS) {}
		virtual ~ClassStatement() {}

	};

	struct MemberFunctionStatement : public FunctionStatement
	{
		MemberFunctionStatement() : FunctionStatement() {}
		virtual ~MemberFunctionStatement() {}

	};


}