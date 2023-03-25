
#pragma once

#include "ast.h"

namespace caliburn
{
	struct VariableStatement : public Statement
	{
		std::vector<Token*> names;
		bool isConst = false;
		ParsedType* typeHint = nullptr;
		Value* initialValue = nullptr;

		VariableStatement(Statement* parent) : Statement(StatementType::VARIABLE, parent) {}
		virtual ~VariableStatement() {}

		void declSymbols() override
		{
			for (auto name : names)
			{
				parent->vars.push_back(Variable(this->parent, name, typeHint, initialValue));

			}

		}

		void resolveSymbols() override {}

	};

}
