
#pragma once

#include "ast.h"

namespace caliburn
{
	class RootStatement : public Statement
	{
	public:
		std::vector<Token*> imports;
		std::vector<Statement*> children;

		RootStatement() : Statement(StatementType::ROOT, nullptr) {}
		virtual ~RootStatement() {}

		Token* firstTkn() const override
		{
			return nullptr;
		}

		Token* lastTkn() const override
		{
			return nullptr;
		}

	};

}
