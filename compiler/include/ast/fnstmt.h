
#pragma once

#include "ast.h"
#include "fn.h"

namespace caliburn
{
	struct FunctionStatement : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Function> fn = nullptr;

		FunctionStatement() : Statement(StatementType::FUNCTION) {}

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
			//table->add(name->str, fn);

		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		/*
		ValidationData validate(ref<const std::set<StatementType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			std::set<StatementType> bodyTypes = LOGIC_STMT_TYPES;
			std::set<ReturnMode> bodyModes = { ReturnMode::RETURN };

			bodyTypes.insert(types.begin(), types.end());
			bodyModes.insert(retModes.begin(), retModes.end());

			return body->validate(bodyTypes, bodyModes);
		}
		*/
		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

}
