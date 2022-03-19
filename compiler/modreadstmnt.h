
#pragma once

#include "valuestmnt.h"

namespace caliburn
{
	struct ModuleReadStatement : public ValueStatement
	{
		Token* const mod;
		ValueStatement* const child;

		ModuleReadStatement(Token* m, ValueStatement* c) : ValueStatement(c->type), mod(m), child(c) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* table) override
		{
			SymbolTable* foundMod = nullptr;
			auto sym = table->resolve(mod->str);

			if (sym)
			{
				if (sym->symbolType != SymbolType::MODULE)
				{
					//TODO complain
					return 0;
				}

				foundMod = ((ModuleSymbol*)sym)->table;

			}
			else
			{
				foundMod = codeAsm->getModule(mod->str);

			}

			if (!foundMod)
			{
				//TODO complain
				return 0;
			}

			return child->SPIRVEmit(codeAsm, foundMod);
		}

		bool isLValue()
		{
			return child->isLValue();
		}

		TypedSSA getSSA()
		{
			return child->getSSA();
		}

	};

}
