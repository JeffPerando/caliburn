
#pragma once

#include "ast.h"

namespace caliburn
{
	struct TypedefStatement : public Statement
	{
		const sptr<Token> first;
		const sptr<Token> name;
		const uptr<ParsedType> alias;

		bool isStrong = false;

		TypedefStatement(sptr<Token> f, sptr<Token> n, uptr<ParsedType> t) : Statement(StatementType::TYPEDEF), first(f), name(n), alias(std::move(t))
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

		virtual void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			auto sym = table->find(name->str);

			if (!std::holds_alternative<nullptr_t>(sym))
			{
				//I don't know what to do for an error of this kind
				//This would've been defered to validation, but that would come with complications
				if (std::holds_alternative<sptr<Type>>(sym))
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
			auto cType = alias->resolve(table);

			if (cType == nullptr)
			{
				//TODO complain
				return;
			}

			table->add(name->str, cType);

		}

		virtual void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			
		}

		virtual void declareHeader(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{

		}

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{

		}

	};

}