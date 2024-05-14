
#pragma once

#include "ast/ast.h"

namespace caliburn
{
	struct TypedefStmt : Statement
	{
		sptr<const CompilerSettings> settings;

		sptr<Token> first;
		sptr<Token> name;
		sptr<ParsedType> alias;

		bool isStrong = false;
		
		TypedefStmt(sptr<const CompilerSettings> cs, sptr<Token> f, sptr<Token> n, sptr<ParsedType> t) :
			Statement(StmtType::TYPEDEF), settings(cs), first(f), name(n), alias(t)
		{
			isStrong = (first->str == "strong");
		}

		virtual ~TypedefStmt() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return alias->lastTkn();
		}

		void declareHeader(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override
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
				auto outTypeName = settings->dynTypes.find(std::string(name->str));

				if (outTypeName != settings->dynTypes.end())
				{
					alias = new_sptr<ParsedType>(outTypeName->second);
				}
				else //No default provided
				{
					if (alias->genericArgs->empty())
					{
						err.err("Dynamic type generic is empty or not present", *alias);
						return;
					}

					if (auto t = alias->genericArgs->getType(0))
					{
						alias = t;
					}
					else //Generic at index 0 is not a type
					{
						err.err("Dynamic type default not found", *alias);
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
				if (!table->add(name->str, t))
				{
					err.err({ "Duplicate type:", name->str }, name);
				}

			}
			else
			{
				err.err({ "Unable to resolve type", alias->name }, *alias);
			}

		}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

	};

}
