
#pragma once

#include "ast.h"
#include "var.h"

namespace caliburn
{
	struct LocalVarStatement : public Statement
	{
		std::vector<sptr<Token>> names;

		sptr<Token> start = nullptr;
		sptr<ParsedType> typeHint = nullptr;
		sptr<Value> initialValue = nullptr;
		bool isConst = false;

	private:
		std::vector<sptr<LocalVariable>> vars;

	public:
		LocalVarStatement() : Statement(StatementType::VARIABLE){}
		virtual ~LocalVarStatement() {}

		sptr<Token> firstTkn() const override
		{
			return nullptr;
		}

		sptr<Token> lastTkn() const override
		{
			return nullptr;
		}

		void declareSymbols(sptr<SymbolTable> table) override
		{
			if (!typeHint->resolve(table))
			{
				//TODO complain
			}

			for (auto const& name : names)
			{
				vars.push_back(new_sptr<LocalVariable>(mods, start, name, typeHint, initialValue));

			}

			for (auto const& v : vars)
			{
				table->add(v->name->str, v);
			}

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable>, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

	};

}
