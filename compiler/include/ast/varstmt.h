
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
			if (vars.empty())
			{
				for (auto const& name : names)
				{
					//mods, start, name, typeHint, initialValue
					auto v = new_sptr<LocalVariable>();

					v->mods = mods;
					v->start = start;
					v->nameTkn = name;
					v->typeHint = typeHint;
					v->initValue = initialValue;
					v->isConst = isConst;

					vars.push_back(v);

				}

			}
			
			for (auto const& v : vars)
			{
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
