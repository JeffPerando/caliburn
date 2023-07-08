
#pragma once

#include "type.h"

namespace caliburn
{
	class TypeVoid;

	class RealVoid : RealType
	{
	public:
		RealVoid(ptr<TypeVoid> parent) : RealType((ptr<BaseType>)parent) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	class TypeVoid : public PrimitiveType<RealVoid>
	{
	public:
		TypeVoid() : PrimitiveType(TypeCategory::VOID, "void", 0, new_sptr<RealVoid>(this)) {}

	};

}
