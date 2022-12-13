
#pragma once

#include "ast.h"

namespace caliburn
{
	class ShaderStatement : public Statement
	{
	public:
		std::vector<void*> stages;

		ShaderStatement(Statement* parent) : Statement(StatementType::SHADER, parent) {}
		virtual ~ShaderStatement() {}



	};

}
