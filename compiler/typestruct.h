
#pragma once

#include "type.h"
#include "valuestmnt.h"

namespace caliburn
{
	struct TypeStruct : public ConcreteType
	{
		TypeStruct(Token* name, size_t genMax) :
			ConcreteType(TypeCategory::STRUCT, name->str, {}, genMax) {}

	};

}
