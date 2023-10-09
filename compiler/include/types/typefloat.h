
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeFloat;

	struct RealFloat : RealType
	{
		RealFloat(ptr<TypeFloat> parent) : RealType((ptr<BaseType>)parent) {}

		sptr<cllr::LowType> emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct TypeFloat : PrimitiveType
	{
		TypeFloat(uint32_t bits) : PrimitiveType(TypeCategory::FLOAT, "float" + bits, bits, new_sptr<RealFloat>(this)) {}

	};

}
