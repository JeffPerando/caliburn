
#pragma once

#include "type.h"

namespace caliburn
{
	class TypeBool;

	class RealBool : RealType
	{
	public:
		RealBool(ptr<TypeBool> parent) : RealType((ptr<BaseType>)parent) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	class TypeBool : public PrimitiveType<RealBool>
	{
	public:
		TypeBool() : PrimitiveType(TypeCategory::BOOLEAN, "bool", 8, new_sptr<RealBool>(this)) {}

	};

}
