
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ValueStatement : public Statement
	{
		ValueStatement(StatementType t) : Statement(t) {}
		virtual ~ValueStatement() {}

		virtual bool isLValue() = 0;

		virtual TypedSSA getSSA() = 0;

	};

}
