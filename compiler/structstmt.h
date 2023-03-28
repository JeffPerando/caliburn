
#pragma once

#include <vector>

#include "ast.h"

namespace caliburn
{
	struct StructStatement : public Statement
	{
		std::vector<Variable*> members;

		StructStatement(Statement* parent) : Statement(StatementType::STRUCT, parent) {}
		StructStatement(StatementType type, Statement* parent) : Statement(type, parent) {}



	};

}
