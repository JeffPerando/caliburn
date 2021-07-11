
#pragma once

#include "statement.h"
#include "valuestmnt.h"

namespace caliburn
{
	struct AssignStatement : public Statement
	{
		std::string field = "";
		ValueStatement* value = nullptr;

		AssignStatement() : Statement(0) {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			if (!value)
			{
				//TODO complain
			}

			auto valueSSA = value->toSPIRV(codeAsm);
			auto ssa = codeAsm->newAssign();

			codeAsm->pushVarSetter(field, valueSSA);

			return 0;
		}

	};

}
