
#pragma once

#include "statement.h"
#include "valuestmnt.h"

namespace caliburn
{
	/*
	//TODO rework
	struct AssignStatement : public Statement
	{
		std::string field = "";
		ValueStatement* value = nullptr;

		AssignStatement() : Statement(0) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm)
		{
			if (!value)
			{
				//TODO complain
			}

			auto valueSSA = value->SPIRVEmit(codeAsm);
			auto ssa = codeAsm->newAssign();

			codeAsm->pushVarSetter(field, valueSSA);

			return 0;
		}

	};
	*/
}
