
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeInt : BaseType
	{
		const uint32_t width;
		const bool isSigned;

		TypeInt(uint32_t bits, bool sign) : BaseType(TypeCategory::INT, (isSigned ? "int" : "uint") + std::to_string(bits)), width(bits), isSigned(sign) {}

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			auto const typeOp = (isSigned ? cllr::Opcode::TYPE_INT_SIGN : cllr::Opcode::TYPE_INT_UNSIGN);

			return codeAsm.pushType(cllr::Instruction(typeOp, { width }));
		}

	};

}
