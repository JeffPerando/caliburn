
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVoid : BaseType
	{
		TypeVoid() : BaseType(TypeCategory::VOID, "void") {}

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			return codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_VOID));
		}

	};

}
