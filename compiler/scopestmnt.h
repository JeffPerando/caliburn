
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ScopeStatement : public ContextStmnt
	{
		std::vector<Statement*> innerCode;
		ScopeStatement() : ContextStmnt(0) {}
		~ScopeStatement() { Statement::~Statement(); }

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			return 0;
		}

	};

}
