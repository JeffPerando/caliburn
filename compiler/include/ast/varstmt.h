
#pragma once

#include "ast.h"
#include "var.h"

namespace caliburn
{
	struct VarStmt : Statement
	{
		sptr<Token> first = nullptr;
		std::vector<sptr<Variable>> vars;

	public:
		VarStmt() : Statement(StmtType::VARIABLE) {}
		virtual ~VarStmt() {}

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

		void emitCodeCLLR(sptr<SymbolTable>, out<cllr::Assembler> codeAsm) override {}

	};

}
