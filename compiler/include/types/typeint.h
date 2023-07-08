
#pragma once

#include "type.h"

namespace caliburn
{
	class TypeInt;

	class RealInt : RealType
	{
	public:
		RealInt(ptr<TypeInt> parent) : RealType((ptr<BaseType>)parent) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	class TypeInt : public PrimitiveType<RealInt>
	{
	public:
		const bool isSigned;

		TypeInt(uint32_t bits, bool sign) : PrimitiveType(TypeCategory::INT, "int" + bits, bits, new_sptr<RealInt>(this)), isSigned(sign) {}

	};

}
