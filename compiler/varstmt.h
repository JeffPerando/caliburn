
#pragma once

#include "ast.h"
#include "var.h"

namespace caliburn
{
	struct LocalVarStatement : public Statement
	{
		std::vector<Token*> names;
		bool isConst = false;
		ParsedType* typeHint = nullptr;
		Value* initialValue = nullptr;

	private:
		std::vector<LocalVariable*> vars;

	public:
		LocalVarStatement() : Statement(StatementType::VARIABLE){}

		virtual ~LocalVarStatement()
		{
			for (auto var : vars)
			{
				delete var;
			}

			vars.clear();

		}

		void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override
		{
			for (auto name : names)
			{
				vars.push_back(new LocalVariable(name, typeHint, initialValue, isConst));

			}

			for (auto v : vars)
			{
				table.add(v->name->str, SymbolType::VARIABLE, v);
			}

		}

		void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override {}

	};

}
