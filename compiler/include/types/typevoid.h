
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVoid : public Type
	{
		TypeVoid() : Type(TypeCategory::VOID, "") {}

		uint32_t getSizeBytes() const override
		{
			return 0;
		}

		uint32_t getAlignBytes() const override
		{
			return 0;
		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override;

	};
}
