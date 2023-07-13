
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeBool;

	struct RealBool : RealType
	{
		RealBool(ptr<TypeBool> parent) : RealType((ptr<BaseType>)parent) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	struct TypeBool : PrimitiveType
	{
		TypeBool() : PrimitiveType(TypeCategory::BOOLEAN, "bool", 8, new_sptr<RealBool>(this)) {}

	};

}
