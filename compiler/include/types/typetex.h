
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeTexture : BaseType
	{
	private:
		const TextureKind kind;

		const GenericSignature genSig = GenericSignature({
			GenericName{GenericSymType::TYPE, "Pixel", new_sptr<ParsedType>("vec3")}
		});

	public:
		TypeTexture(TextureKind tk) : BaseType(TypeCategory::TEXTURE, std::string("tex").append(TEX_TYPES.at(tk))), kind(tk) {}
		virtual ~TypeTexture() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

}