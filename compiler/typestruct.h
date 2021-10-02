
#pragma once

#include "type.h"
#include "valuestmnt.h"

namespace caliburn
{
	struct TypeStruct : public CompiledType
	{
		TypeStruct(Token* name, TypeAttrib attribs, size_t genMax) :
			CompiledType(TypeCategory::CUSTOM, name->str, (TypeAttrib)(attribs | TypeAttrib::COMPOSITE), genMax) {}

	};

}
