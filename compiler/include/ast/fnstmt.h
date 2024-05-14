
#pragma once

#include "ast.h"
#include "fn.h"

namespace caliburn
{
	struct FnStmt : Statement
	{
		const Token first;
		const Token name;
		const sptr<SrcFn> fn;

		FnStmt(out<ParsedFn> pfn) : Statement(StmtType::FUNCTION),
			first(pfn.first), name(pfn.name), fn(new_sptr<SrcFn>(pfn))
		{}

		virtual ~FnStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return fn->code->lastTkn();
		}

		void declareHeader(sptr<SymbolTable> table, out<ErrorHandler> err) override
		{
			sptr<FunctionGroup> group = nullptr;
			auto sym = table->find(name.str);

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

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

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
