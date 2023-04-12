
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

SSA Assembler::push(SSA ssa, Opcode op, std::array<uint32_t, 3> operands, std::array<uint32_t, 3> refs)
{
	/* NOTE: ID == 0 is not an error
	* but making a version of this method that assumes as such is probably a good idea
	if (ssa == 0)
	{
		//TODO complain
		return 0;
	}
	*/
	code.push_back(Instruction{ssa, op, operands, refs});

	return ssa;
}

void Assembler::findRefs(SSA id, std::vector<Instruction*>& result)
{
	for (auto& op : code)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			if (op.refs[i] == id)
			{
				result.push_back(&op);
				break;
			}

		}

	}

}
