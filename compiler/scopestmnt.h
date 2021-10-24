
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ScopeStatement : public ContextStmnt
	{
		std::vector<Statement*> innerCode;
		ScopeStatement() : ContextStmnt(StatementType::SCOPE) {}
		~ScopeStatement() { Statement::~Statement(); }

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			for (auto stmt : innerCode)
			{
				stmt->SPIRVEmit(codeAsm, syms);

			}

			return 0;
		}

	};

}
