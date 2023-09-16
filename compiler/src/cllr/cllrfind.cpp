
#include <algorithm>

#include "cllr/cllrfind.h"

using namespace caliburn::cllr;

ptr<Finder> Finder::ops(in<std::vector<Opcode>> xs)
{
	opcodes.insert(xs.end(), xs.begin(), xs.end());

	std::sort(opcodes.begin(), opcodes.end());

	return this;
}

ptr<Finder> Finder::operands(in<std::array<uint32_t, 3>> xs)
{
	op_args.assign(xs.begin(), xs.end());

	return this;
}

ptr<Finder> Finder::references(SSA id)
{
	refID = id;

	return this;
}

ptr<Finder> Finder::outputs(SSA type)
{
	outType = type;

	return this;
}

ptr<Finder> Finder::setOffset(size_t i)
{
	offset = (uint32_t)i;

	return this;
}

ptr<Finder> Finder::setLimit(uint32_t max)
{
	limit = max;

	return this;
}

InstructionVec Finder::find(in<InstructionVec> code)
{
	if (opcodes.empty() && op_args.empty() && refID == 0 && outType == 0)
	{
		//TODO complain
	}

	InstructionVec found;

	for (size_t x = offset; x < code.size(); ++x)
	{
		if (found.size() == limit)
		{
			break;
		}

		auto const& i = code[x];

		if (!opcodes.empty() && !std::binary_search(opcodes.begin(), opcodes.end(), i.op))
		{
			continue;
		}

		if (!op_args.empty())
		{
			bool valid = true;

			for (uint32_t opInx = 0; opInx < 3; ++opInx)
			{
				if (i.operands[opInx] != op_args[opInx])
				{
					valid = false;
					break;
				}

			}

			if (!valid)
			{
				continue;
			}

		}

		if (refID != 0)
		{
			bool found = false;

			for (SSA inRefID : i.refs)
			{
				if (inRefID == 0)
				{
					continue;
				}

				if (inRefID == refID)
				{
					found = true;
					break;
				}

			}

			if (!found)
			{
				continue;
			}

		}

		if (outType != 0 && outType != i.outType)
		{
			continue;
		}

		found.push_back(i);

	}

	return found;
}
