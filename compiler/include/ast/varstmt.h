
#pragma once

#include "ast.h"
#include "var.h"

namespace caliburn
{
	struct VariableStatement : Statement
	{
		sptr<Token> first = nullptr;
		std::vector<sptr<Variable>> vars;

	public:
		VariableStatement() : Statement(StatementType::VARIABLE) {}
		virtual ~VariableStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return vars.back()->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table) override
		{
			for (auto const& v : vars)
			{
				v->mods = mods;
				table->add(v->nameTkn->str, v);
			}

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable>, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

	};

}
