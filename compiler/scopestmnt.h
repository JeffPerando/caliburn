
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ScopeStatement : public ContextStmnt
	{
		std::vector<Statement*> innerCode;
		ScopeStatement() : ContextStmnt(StatementType::SCOPE) {}
		~ScopeStatement() { Statement::~Statement(); }

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			codeAsm->startScope(0);

			for (auto stmt : innerCode)
			{
				stmt->toSPIRV(codeAsm);

			}

			codeAsm->endScope();

			return 0;
		}

	};

}
