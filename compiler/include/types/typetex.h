
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeTex2D : BaseType
	{
		TypeTex2D() : BaseType(TypeCategory::TEXTURE, "tex2D") {}
		virtual ~TypeTex2D() {}

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

}