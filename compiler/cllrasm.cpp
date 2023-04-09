
#include "cllrasm.h"

using namespace caliburn::cllr;

uint32_t Assembler::addString(std::string str)
{
	auto index = strs.size();

	strs.push_back(str);

	return (uint32_t)index;
}

SSA Assembler::createSSA(Opcode op)
{
	ssas.push_back(nextSSA);

	auto i = ssas[nextSSA];

	++nextSSA;

	return i;
}

SSA Assembler::push(SSA ssa, Opcode op, std::array<uint32_t, 3> operands, std::array<uint32_t, 3> refs, bool genSSA)
{
	if (ssa == 0 && !genSSA)
	{
		//TODO complain
		return 0;
	}

	if (genSSA)
	{
		ssa = createSSA(op);
	}

	code.push_back(Instruction{ssa, op, operands, refs});

	return ssa;
}
