
#pragma once

#include <iostream>

#include "ast.h"

namespace caliburn
{
	struct TypedefStatement : Statement
	{
		sptr<const CompilerSettings> settings;

		sptr<Token> first;
		sptr<Token> name;
		sptr<ParsedType> alias;

		bool isStrong = false;
		
		TypedefStatement(sptr<const CompilerSettings> cs, sptr<Token> f, sptr<Token> n, sptr<ParsedType> t) :
			Statement(StatementType::TYPEDEF), settings(cs), first(f), name(n), alias(t)
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

		void declareHeader(sptr<SymbolTable> table) override {}

		void declareSymbols(sptr<SymbolTable> table) override
		{
			//TODO implement strong typing (needs wrapper)
			//TODO implement error handling for header declaration and QUIT USING STD::COUT

			//We don't want to override an existing symbol
			if (table->has(name->str))
			{
				//TODO complain
				return;
			}

			if (alias->name == "dynamic")
			{
				auto outTypeName = settings->dynTypes.find(name->str);

				if (outTypeName != settings->dynTypes.end())
				{
					alias = new_sptr<ParsedType>(outTypeName->second);
				}
				else //No default provided
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
						return;
					}

				}

			}

			/*
			At this stage in compilation, the only types in the symbol table are BaseTypes.
			Therefore, we don't need to worry about full resolving
			*/
			if (auto t = alias->resolveBase(table))
			{
				table->add(name->str, t);
			}
			else
			{
				std::cout << "was unable to resolve " << alias->name << '\n';
			}

		}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

}
