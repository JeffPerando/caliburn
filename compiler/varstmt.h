
#pragma once

#include "ast.h"
#include "var.h"

namespace caliburn
{
	struct LocalVarStatement : public Statement
	{
		std::vector<sptr<Token>> names;
		bool isConst = false;
		uptr<ParsedType> typeHint = nullptr;
		uptr<Value> initialValue = nullptr;

	private:
		std::vector<sptr<LocalVariable>> vars;

	public:
		LocalVarStatement() : Statement(StatementType::VARIABLE){}

		virtual ~LocalVarStatement() {}

		void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			for (auto name : names)
			{
				vars.push_back(std::make_unique<LocalVariable>(name, typeHint, initialValue, isConst));

			}

			for (auto const& v : vars)
			{
				table->add(v->name->str, SymbolType::VARIABLE, v);
			}

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

	};

}
