
#pragma once

#include "type.h"

namespace caliburn
{
	struct RealVector : RealType
	{
		RealVector(ptr<Generic<RealVector>> parent, sptr<GenericArguments> gArgs) : RealType((ptr<BaseType>)parent, gArgs) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	struct TypeVector : GenericType<RealVector>
	{
		const uint32_t elements;

		TypeVector(uint32_t vecElements) :
			GenericType(TypeCategory::VECTOR, "vec" + vecElements,
				new_sptr<GenericSignature>(std::initializer_list{
					GenericName(GenericSymType::TYPE, "T", GenericResult(new_sptr<ParsedType>("float32")))
				})
			),
			elements(vecElements)
		{}

	};

}