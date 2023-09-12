
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVector;

	struct RealVector : RealType
	{
		const uint32_t length;

		RealVector(ptr<TypeVector> parent, sptr<GenericArguments> gArgs, uint32_t l) : RealType((ptr<BaseType>)parent, gArgs), length(l) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	struct TypeVector : GenericType<RealVector>
	{
		const uint32_t elements;

		TypeVector(uint32_t vecElements) :
			GenericType(TypeCategory::VECTOR, "vec" + vecElements,
				new_sptr<GenericSignature>(std::initializer_list{
					GenericName(GenericSymType::TYPE, "T", GenericResult(new_sptr<ParsedType>("float32")))
				}),
				lambda_v(sptr<GenericArguments> gArgs) {
					return new_sptr<RealVector>(this, gArgs, vecElements);
				}
			),
			elements(vecElements)
		{}

		virtual Member getMember(ref<const std::string> name) const override;

	};

}