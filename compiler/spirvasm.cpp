
#include "spirvasm.h"

using namespace caliburn::spirv;

SSA CodeSection::push(SpvOp op, std::initializer_list<uint32_t> args, bool genSSA)
{
	if (!validOps.empty())
	{
		if (!std::binary_search(validOps.begin(), validOps.end(), op))
		{
			return 0;
		}

	}

	SSA ssa = 0;

	code.push_back(op);

	if (genSSA)
	{
		ssa = codeAsm->createSSA(op);
		code.push_back(ssa);
	}

	code.insert(code.end(), args.begin(), args.end());

	return ssa;
}

//Do NOT try to replace this with a memcpy
void CodeSection::pushStr(std::string str)
{
	bool needsExtraNull = ((str.length() & 0x3) == 0);
	size_t packedLen = (str.length() >> 2) + needsExtraNull;
	code.reserve(code.size() + packedLen);

	auto iter = str.begin();

	while (iter != str.end())
	{
		uint32_t packed = 0;

		for (auto i = 0; i < 4; ++i)
		{
			if (iter == str.end())
			{
				break;
			}

			packed |= (((uint32_t)*iter) << i * 8);
			++iter;

		}

		code.push_back(packed);

	}

	if (needsExtraNull)
	{
		code.push_back(0);
	}

}

SSA Assembler::createSSA(SpvOp op)
{
	auto entry = SSAEntry { ssa, op };

	++ssa;

	ssaEntries.push_back(entry);

	return entry.ssa;
}

void Assembler::addExt(std::string ext)
{
	extensions.push_back(ext);
}
