
#pragma once

#include <iostream>

#include "ast.h"

namespace caliburn
{
	struct TypedefStatement : Statement
	{
		sptr<CompilerSettings> cs;

		sptr<Token> first;
		sptr<Token> name;
		sptr<ParsedType> alias;

		bool isStrong = false;
		
		TypedefStatement(sptr<CompilerSettings> sets, sptr<Token> f, sptr<Token> n, sptr<ParsedType> t) :
			Statement(StatementType::TYPEDEF), cs(sets), first(f), name(n), alias(t)
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

		void declareHeader(sptr<SymbolTable> table) override
		{
			//TODO implement strong typing (needs wrapper)

			//We're a dynamic typedef
			if (alias->name == "dynamic")
			{
				auto outTypename = cs->dynTypes.find(name->str);

				if (outTypename != cs->dynTypes.end())
				{
					//In both cases we replace alias
					alias = new_sptr<ParsedType>(outTypename->second);
					//No default provided
					
				}
				else
				{
					if (alias->genericArgs->empty())
					{
						std::cout << "dynamic type generic is empty\n";
						return;
					}

					if (auto t = alias->genericArgs->getType(0))
					{
						alias = t;
					}
					else //Generic at index 0 is not a type
					{
						std::cout << "dynamic type default not found\n";
						//TODO complain
						return;
					}

				}
				
			}
			
			if (auto t = alias->resolve(table))
			{
				table->add(name->str, t);
			}
			else
			{
				std::cout << "was unable to resolve " << alias->name << '\n';
			}

		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

	};

}
