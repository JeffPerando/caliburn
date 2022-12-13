
#pragma once

#include "ast.h"

namespace caliburn
{
	class ScopeStatement : public Statement
	{
	public:
		Token *first, *last;

		ScopeStatement(Statement* parent) : Statement(StatementType::SCOPE, parent) {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			return last;
		}

		virtual cllr::SSA toCLLR(cllr::Assembler& codeAsm) override
		{
			return 0;
		}

	};

}
