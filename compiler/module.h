
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

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) = 0;

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) = 0;

		virtual bool validateModule() const = 0;

	};

}
