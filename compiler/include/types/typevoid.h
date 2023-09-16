
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVoid;

	struct RealVoid : RealType
	{
	public:
		RealVoid(ptr<TypeVoid> parent) : RealType((ptr<BaseType>)parent) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct TypeVoid : PrimitiveType
	{
	public:
		TypeVoid() : PrimitiveType(TypeCategory::VOID, "void", 0, new_sptr<RealVoid>(this)) {}

	};

}
