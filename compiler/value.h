
#pragma once

#include "spirvasm.h"

namespace caliburn
{
	struct Value
	{
		virtual bool isLValue() { return false; }

		virtual CompiledType* getType() = 0;

		virtual TypedSSA emitSPIRV(SpirVAssembler* codeAsm, SymbolTable* syms) = 0;

	};

	struct ArithmeticValue : Value
	{
		Value* lValue = nullptr;
		Value* rValue = nullptr;
		Operator op;

		TypedSSA emitSPIRV(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			auto lSSA = lValue->emitSPIRV(codeAsm, syms);
			auto rSSA = rValue->emitSPIRV(codeAsm, syms);

			auto status = lSSA.type->isCompatible(op, rSSA.type);

			if (status != TypeCompat::COMPATIBLE)
			{
				//TODO complain
				
			}

		}

	};

}