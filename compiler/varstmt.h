
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
			for (auto const& name : names)
			{
				//TODO problematic lifetime
				//vars.push_back(std::make_unique<LocalVariable>(name, typeHint, initialValue, isConst));

			}

			for (auto const& v : vars)
			{
				table->add(v->name->str, v);
			}

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override {}

	};

}
