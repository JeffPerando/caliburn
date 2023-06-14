
#include <algorithm>

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
	ssaToOpcode.push_back(op);
	ssaRefs.push_back(0);

	auto i = nextSSA;

	++nextSSA;

	return i;
}

SSA Assembler::push(SSA ssa, Opcode op, std::array<uint32_t, 3> operands, std::array<uint32_t, 3> refs)
{
	//NOTE: ID == 0 is not an error

	if (ssaToOpcode[ssa] != op)
	{
		//TODO complain
	}

	auto ins = new_sptr<Instruction>();

	ins->index = ssa;
	ins->op = op;
	ins->operands = operands;
	ins->refs = refs;

	code.push_back(ins);

	//if (ssa != 0) ssaToCode.push_back(ins);

	for (auto i = 0; i < 3; ++i)
	{
		if (refs[i] != 0)
		{
			ssaRefs[refs[i]] += 1;
		}

	}

	return ssa;
}

void Assembler::findRefs(SSA id, ref<InstructionVec> result, size_t off) const
{
	for (size_t i = off; i < code.size(); ++i)
	{
		auto const& op = code[i];
		for (size_t i = 0; i < 3; ++i)
		{
			if (op->refs[i] == id)
			{
				result.push_back(op);
				break;
			}

		}

	}

}

void Assembler::findPattern(ref<InstructionVec> result,
	Opcode opcode,
	std::array<bool, 3> opFlags, std::array<uint32_t, 3> ops,
	std::array<bool, 3> refFlags, std::array<SSA, 3> refs,
	size_t off, size_t limit) const
{
	size_t count = 0;

	for (size_t i = off; i < code.size(); ++i)
	{
		auto const& ins = code[i];
		if (count == limit)
		{
			break;
		}

		//we treat UNKNOWN as a wildcard value
		if (opcode != Opcode::UNKNOWN && ins->op != opcode)
		{
			continue;
		}

		bool match = true;

		for (auto i = 0; i < 3; ++i)
		{
			if (opFlags[i])
			{
				if (ins->operands[i] != ops[i])
				{
					match = false;
					break;
				}

			}

			if (refFlags[i])
			{
				if (ins->refs[i] != refs[i])
				{
					match = false;
					break;
				}

			}

		}

		if (match)
		{
			result.push_back(ins);
			++count;
		}

	}

}

void Assembler::findAll(ref<InstructionVec> result, const std::vector<Opcode> ops, size_t off, size_t limit) const
{
	if (limit == 0)
	{
		return;
	}

	size_t count = 0;

	for (size_t i = off; i < code.size(); ++i)
	{
		auto const& in = code[i];
		if (std::binary_search(ops.begin(), ops.end(), in->op))
		{
			result.push_back(in);
			++count;

			if (count == limit)
			{
				break;
			}

		}

	}

}

uint32_t Assembler::replace(SSA in, SSA out)
{
	if (in == 0)
	{
		return 0;//TODO consider throwing exception
	}

	uint32_t count = 0;
	uint32_t limit = ssaRefs[in];

	for (auto const& op : code)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			if (op->refs[i] == in)
			{
				op->refs[i] = out;
				++count;

			}

		}

		if (count == limit)
		{
			break;
		}

	}

	ssaRefs[in] = 0;
	
	if (out != 0)
	{
		ssaRefs[out] += count;
	}

	return count;
}
