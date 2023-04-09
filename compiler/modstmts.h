
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ImportStatement : public Statement
	{
		const ptr<Token> first;
		Token* name = nullptr;
		Token* alias = nullptr;

		ImportStatement(Token* f) : Statement(StatementType::IMPORT), first(f) {}
		virtual ~ImportStatement() {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			if (alias == nullptr)
			{
				return name;
			}

			return alias;
		}

	};

	struct ModuleStatement : public Statement
	{
		const ptr<Token> first, name;

		ModuleStatement(Token* s, Token* n) : Statement(StatementType::MODULE), first(s), name(n) {}
		virtual ~ModuleStatement() {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			return name;
		}

	};

}
