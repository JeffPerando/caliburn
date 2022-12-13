
#include "cllrasm.h"

using namespace caliburn::cllr;

uint32_t Assembler::addString(std::string str)
{
	auto index = strs.size();

	strs.push_back(str);

	return index;
}

SSA Assembler::createSSA(Opcode op)
{
	return 0;
}

SSA Assembler::push(Opcode op, std::initializer_list<uint32_t> operands, bool genSSA)
{
	if (operands.size() > 3)
	{
		//TODO complain
	}

	Instruction i;

	i.op = op;

	if (genSSA)
	{
		ssas.push_back(nextSSA);

		i.index = &ssas[nextSSA];

		++nextSSA;

	}

	uint32_t argIndex = 0;

	for (auto arg = operands.begin(); arg != operands.end(); ++arg)
	{
		i.operands[argIndex] = *arg;

	}

	code.push_back(i);

	return 0;
}
