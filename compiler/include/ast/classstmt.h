
#pragma once

#include "fnstmt.h"
#include "structstmt.h"

namespace caliburn
{
	struct ClassStatement : public StructStatement
	{
		ClassStatement(sptr<Token> f) : StructStatement(f, StatementType::CLASS) {}
		virtual ~ClassStatement() {}

	};


}