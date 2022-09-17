
#include "spirvasm.h"

using namespace caliburn::spirv;

SSA CodeSection::push(SpvOp op, std::initializer_list<uint32_t> args, bool hasSSA)
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

	if (hasSSA)
	{
		ssa = codeAsm->nextSSA(op);
		code.push_back(ssa);
	}

	code.insert(code.end(), args.begin(), args.end());

	return ssa;
}

void CodeSection::pushStr(std::string str)
{
	bool needsExtraNull = ((str.length() & 0x3) != 0);
	size_t packedLen = (str.length() >> 2) + needsExtraNull;
	code.reserve(code.size() + packedLen);

	auto iter = str.begin();

	while (iter != str.end())
	{
		uint32_t packed = 0;

		for (auto i = 0; i < 4; ++i)
		{
			packed <<= 8;
			packed |= *iter;
			++iter;

			if (iter == str.end())
				break;

		}

		code.push_back(packed);

	}

	if (needsExtraNull)
	{
		code.push_back(0);
	}

}

SSA Assembler::nextSSA(SpvOp op)
{
	return SSA();
}

void Assembler::pushExt(std::string ext)
{
	extensions.push_back(ext);
}
