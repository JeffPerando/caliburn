
#pragma once

#include "ast.h"

namespace caliburn
{
	struct TypedefStatement : public Statement
	{
		const ptr<Token> first;
		const ptr<Token> name;
		const ptr<ParsedType> alias;

		bool isStrong = false;

		TypedefStatement(ptr<Token> f, ptr<Token> n, ptr<ParsedType> t) : Statement(StatementType::TYPEDEF), first(f), name(n), alias(t)
		{
			isStrong = (first->str == "strong");
		}

		virtual ~TypedefStatement() {}

		Token* firstTkn() const override
		{
			return first;
		}

		Token* lastTkn() const override
		{
			return alias->lastTkn();
		}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override
		{
			auto sym = table.find(name->str);

			if (sym != nullptr)
			{
				//I don't know what to do for an error of this kind
				//This would've been defered to validation, but that would come with complications
				if (sym->type == SymbolType::TYPE)
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
			//Hi past me, it's present me! Finish the declareHeader system and this will fix itself!
			auto cType = alias->resolve(table);

			if (cType == nullptr)
			{
				//TODO complain
				return;
			}

			table.add(name->str, SymbolType::TYPE, cType);

		}

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override
		{
			
		}

		virtual void emitHeaderCLLR(SymbolTable* table, cllr::Assembler& codeAsm)
		{

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{

		}

	};

}