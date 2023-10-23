
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVector : BaseType
	{
	private:
		GenArgMap<cllr::LowType> variants;
	public:
		const uint32_t elements;
		const uptr<GenericSignature> genSig = new_uptr<GenericSignature>(std::initializer_list{
			GenericName(GenericSymType::TYPE, "T", GenericResult(new_sptr<ParsedType>("fp32")))
			});

		TypeVector(uint32_t vecElements) :
			BaseType(TypeCategory::VECTOR, "vec" + vecElements),
			elements(vecElements)
		{}

		virtual sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

}