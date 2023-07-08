
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

		void declareHeader(sptr<SymbolTable> table) const override {} //Do nothing here

		void declareSymbols(sptr<SymbolTable> table) override
		{
			auto sym = table->find(name->str);

			if (!std::holds_alternative<nullptr_t>(sym))
			{
				//I don't know what to do for an error of this kind
				//This would've been defered to validation, but that would come with complications
				if (std::holds_alternative<sptr<BaseType>>(sym))
				{
					//TODO complain
				}
				else
				{
					//TODO complain
				}

				return;
			}
			
			//We do a little cheating and do a little resolving here; It will probably bite me in the arse, tbh
			//Actually I know it will since the top-level symbols won't all be here
			//Hi present me, it's past me! Finish the declareHeader system and this will fix itself!
			/*
			if (auto t = alias->resolve(table))
			{
				table->add(name->str, t);
			}
			*/

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			
		}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

	};

}