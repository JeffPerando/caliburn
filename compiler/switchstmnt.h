
#pragma once

#include "statement.h"
#include "valuestmnt.h"

namespace caliburn
{
	struct SwitchStatement : public Statement
	{
		ValueStatement* condition = nullptr;
		std::vector<Statement*> cases;

		SwitchStatement() : Statement(StatementType::SWITCH) {}
		
		~SwitchStatement()
		{
			delete condition;

			for (auto c : cases)
			{
				delete c;
			}

		}

		virtual uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			return 0;
		}

	};

	struct CaseStatement : public Statement
	{
		ValueStatement* condition = nullptr;
		std::vector<Statement*> logic;

		CaseStatement() : Statement(StatementType::CASE){}

		~CaseStatement()
		{
			delete condition;
			for (auto l : logic)
			{
				delete l;
			}
		}

		virtual uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			return 0;
		}

	};

	struct PassStatement : public Statement
	{
		PassStatement() : Statement(StatementType::PASS) {}

		virtual uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			return 0;
		}

	};

}
