
#pragma once

#include <string>

#include "symbols.h"

namespace caliburn
{
	/*
	Currently deprecated; a proper module system will be added eventually
	*/
	struct Module
	{
		Module() = default;
		virtual ~Module() {}

		virtual sptr<SymbolTable> getTable() const = 0;

	};

}
