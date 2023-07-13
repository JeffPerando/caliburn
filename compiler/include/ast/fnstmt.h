
#pragma once

#include "ast.h"
#include "fn.h"

namespace caliburn
{
	struct FunctionStatement : public Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<GenericSignature> genSig = nullptr;
		std::vector<sptr<FnArgVariable>> args;
		sptr<ParsedType> retType = nullptr;
		uptr<ScopeStatement> body = nullptr;
		
		FunctionStatement() : Statement(StatementType::FUNCTION) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return body->lastTkn();
		}

		void declareHeader(sptr<SymbolTable> table) override {}

		void declareSymbols(sptr<SymbolTable> table) override
		{
			auto sig = new_sptr<FunctionSignature>();

			sig->args = args;
			sig->genSig = genSig;
			sig->returnType = retType;

			auto fn = new_sptr<Function>(sig);

			fn->code = std::move(body);

			table->add(name->str, fn);

		}

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
		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

	};

}
