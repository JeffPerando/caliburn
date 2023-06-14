
#include "cllrspirv.h"
#include "spirvasm.h"

using namespace caliburn;

void spirv::CodeSection::push(spirv::SpvOp op, SSA id, std::vector<uint32_t> args)
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
		spvAsm->setOpForSSA(id, op);

	}

	code.insert(code.end(), args.begin(), args.end());

}

void spirv::CodeSection::pushRaw(std::vector<uint32_t> args)
{
	code.insert(code.end(), args.begin(), args.end());

}

//Do NOT try to replace this with a memcpy
void spirv::CodeSection::pushStr(std::string str)
{
	code.reserve(code.size() + spirv::SpvStrLen(str));

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

			packed |= (((uint32_t)*iter) << (i * 8));
			++iter;

		}

		code.push_back(packed);

	}

	if ((str.size() & 0x3) == 0)
	{
		code.push_back(0);
	}

}
