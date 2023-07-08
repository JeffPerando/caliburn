
#pragma once

#include "type.h"

namespace caliburn
{
	class TypeVector;

	class RealVector : RealType
	{
		RealVector(ptr<TypeVector> parent, sptr<GenericArguments> gArgs) : RealType((ptr<BaseType>)parent, gArgs) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	class TypeVector : public GenericType<RealVector>
	{
	public:
		const uint32_t elements;

		TypeVector(uint32_t vecElements) :
			GenericType<RealVector>(TypeCategory::VECTOR, "vec" + vecElements,
				new_sptr<GenericSignature>(std::initializer_list{
			GenericName(GenericSymType::TYPE, "T", GenericResult(new_sptr<ParsedType>("float32")))
		})
			),
			elements(vecElements)
		{}

	};

}