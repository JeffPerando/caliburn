
#pragma once

#include <map>

#include "langcore.h"

namespace caliburn
{
	enum class SymbolType
	{
		UNKNOWN,
		STATEMENT,
		VALUE,
		TYPE
	};

	struct Symbol
	{
		const SymbolType type;
		const void* data;

	};

}
