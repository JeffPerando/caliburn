
#include "cllrspirv.h"
#include "spirvasm.h"

using namespace caliburn;

void spirv::CodeSection::push(spirv::SpvOp op, SSA id, std::vector<uint32_t> args)
{
	if (!isValidOp(op))
	{
		return;
	}

	code.push_back(op);

	if (id != 0)
	{
		code.push_back(id);
		spvAsm->setOpForSSA(id, op);

	}

	code.insert(code.end(), args.begin(), args.end());

}

void spirv::CodeSection::pushVal(SpvOp op, SSA type, SSA id, std::vector<uint32_t> args)
{
	//TODO check for valid type, maybe.

	if (!isValidOp(op))
	{
		return;
	}

	if (type == 0)
	{
		return;
	}

	if (id == 0)
	{
		return;
	}

	code.push_back(op);
	code.push_back(type);
	code.push_back(id);
	spvAsm->setOpForSSA(id, op);

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

spirv::SSA spirv::CodeSection::find(SpvOp op, std::vector<uint32_t> args)
{
	size_t i = 0;

	while (i < code.size())
	{
		SpvOp other = SpvOp(code[i]);

		bool valid = false;

		if (op == other)
		{
			valid = false;
		}
		else
		{
			for (size_t off = 0; off < args.size(); ++off)
			{
				if (code[i + 2 + off] != args[off])
				{
					valid = false;
					break;
				}

			}

		}

		if (valid)
		{
			return code[i + 2];
		}

		i += other.words;

	}

	return 0;
}
