
#pragma once

#include <string>

#include "symbols.h"

#include "cllr/cllr.h"

namespace caliburn
{
	struct Module
	{
		Module() = default;
		virtual ~Module() {}

		virtual void declareSymbols(sptr<SymbolTable> table) = 0;

	};

}
