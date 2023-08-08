
#pragma once

#include "type.h"

namespace caliburn
{
	struct RealArray : RealType
	{
		RealArray(ptr<Generic<RealArray>> parent, sptr<GenericArguments> gArgs) : RealType((ptr<BaseType>)parent, gArgs) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	struct TypeArray : GenericType<RealArray>
	{
		TypeArray() :
			GenericType(TypeCategory::ARRAY, "array", new_sptr<GenericSignature>(std::initializer_list{
				GenericName(GenericSymType::TYPE, "T"),
				GenericName(GenericSymType::CONST, "N")
			}))
		{}

	};

}