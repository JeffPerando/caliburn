
#pragma once

#include "fnstmt.h"
#include "structstmt.h"

namespace caliburn
{
	struct ClassStatement : StructStatement
	{
		ClassStatement(sptr<Token> f) : StructStatement(f, StatementType::CLASS) {}
		virtual ~ClassStatement() {}

	};


}