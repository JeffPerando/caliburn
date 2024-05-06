
#pragma once

#include "ast.h"
#include "fn.h"

namespace caliburn
{
	struct FnStmt : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<SrcFn> fn = nullptr;

		FnStmt() : Statement(StmtType::FUNCTION) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return fn->code->lastTkn();
		}

		void declareHeader(sptr<SymbolTable> table) override
		{
			sptr<FunctionGroup> group = nullptr;
			auto sym = table->find(name->str);

			MATCH_EMPTY(sym)
			{
				group = new_sptr<FunctionGroup>();
			}
			else MATCH(sym, sptr<FunctionGroup>, fnGroup)
			{
				group = *fnGroup;
			}
			else
			{
				//TODO complain
			}

			group->add(fn);
			
		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		/*
		ValidationData validate(ref<const std::set<StmtType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			std::set<StmtType> bodyTypes = LOGIC_STMT_TYPES;
			std::set<ReturnMode> bodyModes = { ReturnMode::RETURN };

			bodyTypes.insert(types.begin(), types.end());
			bodyModes.insert(retModes.begin(), retModes.end());

			return body->validate(bodyTypes, bodyModes);
		}
		*/
		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

}
