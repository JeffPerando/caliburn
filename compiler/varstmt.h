
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
		LocalVarStatement(Statement* parent) : Statement(StatementType::VARIABLE, parent){}

		virtual ~LocalVarStatement()
		{
			for (auto var : vars)
			{
				delete var;
			}

			vars.clear();

		}

		void declSymbols(SymbolTable& table) override
		{
			for (auto name : names)
			{
				vars.push_back(new LocalVariable(this->parent, name, typeHint, initialValue, isConst));

			}

			for (auto v : vars)
			{
				table.add(v->name->str, SymbolType::VALUE, v);
			}

		}

		void resolveSymbols(const SymbolTable& table) override {}

	};

}
