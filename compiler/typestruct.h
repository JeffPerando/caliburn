
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeStruct : public Type
	{
		TypeStruct(Token* name, size_t genMax) :
			Type(TypeCategory::STRUCT, name->str, 0, genMax) {}

	};

}
