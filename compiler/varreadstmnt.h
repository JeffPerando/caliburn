
#pragma once

#include "valuestmnt.h"

namespace caliburn
{
	struct VarReadStatement : public ValueStatement
	{
	private:
		TypedSSA value;
	public:
		const std::string label;

		VarReadStatement(std::string l) : ValueStatement(StatementType::UNKNOWN), label(l) {}
		virtual ~VarReadStatement() {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms) override
		{
			auto sym = syms->resolve(label);

			if (!sym)
			{
				//TODO complain
				return 0;
			}

			if (sym->symbolType != SymbolType::VARIABLE)
			{
				//TODO complain
				return 0;
			}

			auto symbol = (VarSymbol*)sym;
			auto ssa = codeAsm->newAssign();
			auto type = symbol->ssa.type->getSSA();

			codeAsm->pushAll({ spirv::OpLoad(0), type, ssa, spirv::SpvOp(symbol->ssa.value) });

			value = TypedSSA(symbol->ssa.type, ssa);

			return ssa;
		}

		bool isLValue() override
		{
			return true;
		}

		TypedSSA getSSA()
		{
			return value;
		}

	};
}
