
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeArray : BaseType
	{
		const uptr<GenericSignature> sig = new_uptr<GenericSignature>(std::vector{
				GenericName(GenericSymType::TYPE, "T"),
				GenericName(GenericSymType::CONST, "N")
			});

		TypeArray() : BaseType(TypeCategory::ARRAY, "array") {}

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

}