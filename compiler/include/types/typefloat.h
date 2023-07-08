
#pragma once

#include "type.h"

namespace caliburn
{
	class TypeFloat;

	class RealFloat : RealType
	{
	public:
		RealFloat(ptr<TypeFloat> parent) : RealType((ptr<BaseType>)parent) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	class TypeFloat : public PrimitiveType<RealFloat>
	{
	public:
		TypeFloat(uint32_t bits) : PrimitiveType(TypeCategory::FLOAT, "float" + bits, bits, new_sptr<RealFloat>(this)) {}

	};

}
