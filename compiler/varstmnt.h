
#pragma once

#include "valuestmnt.h"

namespace caliburn
{
	struct VarReadStatement : public ValueStatement
	{
		Token* var = nullptr;

		VarReadStatement() : ValueStatement(StatementType::UNKNOWN) {}

		~VarReadStatement() {}

		bool isLValue() override
		{
			return true;
		}

	};
}
