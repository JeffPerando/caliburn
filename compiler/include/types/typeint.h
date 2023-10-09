
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeInt;

	struct RealInt : RealType
	{
		RealInt(ptr<TypeInt> parent) : RealType((ptr<BaseType>)parent) {}

		sptr<cllr::LowType> emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct TypeInt : PrimitiveType
	{
		const bool isSigned;

		TypeInt(uint32_t bits, bool sign) : PrimitiveType(TypeCategory::INT, "int" + bits, bits, new_sptr<RealInt>(this)), isSigned(sign) {}

	};

}
