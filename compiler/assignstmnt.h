
#pragma once

#include "statement.h"
#include "valuestmnt.h"

namespace caliburn
{
	//TODO finish
	struct AssignStatement : public Statement
	{
	private:
		const std::string field;
	public:
		ValueStatement* value = nullptr;

		AssignStatement(std::string f) : Statement(StatementType::SETTER), field(f) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			if (!value)
			{
				//TODO complain
				return 0;
			}

			Symbol* sym = syms->resolve(field);

			if (!sym)
			{
				//TODO complain
				return 0;
			}

			if (sym->symbolType == SymbolType::FUNC_PARAM)
			{
				//TODO complain
				return 0;
			}

			auto valueSSA = value->SPIRVEmit(codeAsm, syms);

			if (sym->symbolType == SymbolType::VARIABLE)
			{

			}
			else if (sym->symbolType == SymbolType::MEMBER)
			{

			}

			return valueSSA;
		}

	};

}
