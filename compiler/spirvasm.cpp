
#include "spirvasm.h"

using namespace caliburn::spirv;

void CodeSection::push(SpvOp op, SSA id, std::initializer_list<uint32_t> args)
{
	if (!validOps.empty())
	{
		if (!std::binary_search(validOps.begin(), validOps.end(), op))
		{
			return;
		}

	}

	code.push_back(op);

	if (id != 0)
	{
		code.push_back(id);
	}

	code.insert(code.end(), args.begin(), args.end());

}

void CodeSection::pushRaw(std::initializer_list<uint32_t> args)
{
	code.insert(code.end(), args.begin(), args.end());

}

void CodeSection::pushRaw(std::vector<uint32_t> args)
{
	code.insert(code.end(), args.begin(), args.end());

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
