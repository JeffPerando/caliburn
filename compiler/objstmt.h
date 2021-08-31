
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ClassStatement : public Statement
	{
		Token* name;
		ParsedType* ext = nullptr;
		std::vector<ParsedType*> interfaces;
		std::vector<Statement*> members;
		
		ClassStatement(Token* className) : name(className), Statement(StatementType::CLASS) {}

	};

}
