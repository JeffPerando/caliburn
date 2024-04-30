
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeFloat : BaseType
	{
		const uint32_t width;
		TypeFloat(uint32_t bits) : BaseType(TypeCategory::FLOAT, "fp" + std::to_string(bits)), width(bits) {}

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			return codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_FLOAT, { width }));
		}

	};

}
