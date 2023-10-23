
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeBool : BaseType
	{
		TypeBool() : BaseType(TypeCategory::BOOLEAN, "bool") {}

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			return codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_BOOL));
		}

	};

}
