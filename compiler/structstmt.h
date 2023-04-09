
#pragma once

#include <vector>

#include "ast.h"

namespace caliburn
{
	struct StructStatement : public Statement
	{
		std::vector<Variable*> members;

		StructStatement(StatementType type = StatementType::STRUCT) : Statement(type) {}
		virtual ~StructStatement() {}

		virtual Token* firstTkn() const override
		{
			return nullptr;
		}

		virtual Token* lastTkn() const override
		{
			return nullptr;
		}

		//Only used by top-level statements which declare symbols. The rest, like local variables, should use declareSymbols() instead
		virtual void declareHeader(ref<SymbolTable> table, cllr::Assembler& codeAsm) {}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override = 0;

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override = 0;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) = 0;



	};

}
