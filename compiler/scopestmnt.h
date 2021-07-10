
#pragma once

#include "ctrlstmnt.h"
#include "fieldstmnt.h"
#include "statement.h"

namespace caliburn
{
	struct ScopeStatement : public ContextStmnt
	{
		std::vector<Statement*> innerCode;
		std::vector<FieldStatement*> fields;
		ScopeStatement() : ContextStmnt(0) {}
		~ScopeStatement() { Statement::~Statement(); }

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			return 0;
		}

		uint32_t addField(std::string name, Visibility vis)
		{
			return 0;
		}

		uint32_t resolveType(std::string id)
		{
			return 0;
		}

		uint32_t resolveName(std::string id)
		{
			return 0;
		}

		uint32_t resolveIdentifier(std::string id)
		{
			return 0;
		}

	};

}
