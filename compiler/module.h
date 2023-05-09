
#pragma once

#include <string>

#include "cllr.h"
#include "symbols.h"

namespace caliburn
{
	struct Module
	{
		Module() = default;
		virtual ~Module() {}

		virtual void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;

		virtual void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;

		virtual bool validateModule() const = 0;

	};

}
