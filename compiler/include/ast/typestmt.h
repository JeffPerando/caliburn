
#pragma once

#include "ast.h"

namespace caliburn
{
	struct TypedefStatement : public Statement
	{
		const sptr<Token> first;
		const sptr<Token> name;
		const sptr<ParsedType> alias;

		bool isStrong = false;
		
		TypedefStatement(sptr<Token> f, sptr<Token> n, sptr<ParsedType> t) : Statement(StatementType::TYPEDEF), first(f), name(n), alias(t)
		{
			isStrong = (first->str == "strong");
		}

		virtual ~TypedefStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return alias->lastTkn();
		}

		void declareHeader(sptr<SymbolTable> table) override {} //Do nothing here

		void declareSymbols(sptr<SymbolTable> table) override
		{
			//TODO implement strong typing (needs wrapper)

			table->add(name->str, alias->resolve(table));

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

	};

}