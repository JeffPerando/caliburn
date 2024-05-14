
#pragma once

#include "ast.h"
#include "var.h"

namespace caliburn
{
	struct VarStmt : Statement
	{
		Token first;
		std::vector<sptr<Variable>> vars;

	public:
		VarStmt() : Statement(StmtType::VARIABLE) {}
		virtual ~VarStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return vars.back()->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override
		{
			for (auto const& v : vars)
			{
				v->mods = mods;
				table->add(v->name, v);
			}

		}

		void emitCodeCLLR(sptr<SymbolTable>, out<cllr::Assembler> codeAsm) override {}

	};

}
