
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ExpressionValue : Value
	{
		Value* lValue = nullptr;
		Value* rValue = nullptr;
		Operator op;

	};

}