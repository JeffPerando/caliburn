
#pragma once

#include <string>

#include "cllr/cllr.h"
#include "ast/symbols.h"

namespace caliburn
{
	struct Module
	{
		Module() = default;
		virtual ~Module() {}

		virtual void declareSymbols(sptr<SymbolTable> table) = 0;

		virtual void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;

		virtual bool validateModule() const = 0;

	};

}