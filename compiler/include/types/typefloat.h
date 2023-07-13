
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeFloat;

	struct RealFloat : RealType
	{
		RealFloat(ptr<TypeFloat> parent) : RealType((ptr<BaseType>)parent) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	struct TypeFloat : PrimitiveType
	{
		TypeFloat(uint32_t bits) : PrimitiveType(TypeCategory::FLOAT, "float" + bits, bits, new_sptr<RealFloat>(this)) {}

	};

}
