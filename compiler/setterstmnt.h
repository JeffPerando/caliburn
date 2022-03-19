
#pragma once

#include "valuestmnt.h"

namespace caliburn
{
	struct SetterStatement : public Statement
	{
		ValueStatement* const target;
		ValueStatement* const value;

		SetterStatement(ValueStatement* lhs, ValueStatement* rhs) : Statement(StatementType::SETTER), target(lhs), value(rhs) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms) override
		{
			target->SPIRVEmit(codeAsm, syms);
			value->SPIRVEmit(codeAsm, syms);

			auto lhs = target->getSSA();
			auto rhs = value->getSSA();

			//TODO ensure compatibility

			codeAsm->pushAll({spirv::OpStore(0), spirv::SpvOp(lhs.value), spirv::SpvOp(rhs.value)});

			return 0;
		}

	};

}
