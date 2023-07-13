
#include "spirv/spirvasm.h"

#include "spirv/cllrspirv.h"

using namespace caliburn::spirv;

void CodeSection::push(SpvOp op, SSA id, std::vector<uint32_t> args)
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
		spvAsm->setSection(id, section);

	}

	code.insert(code.end(), args.begin(), args.end());

}

void CodeSection::pushTyped(SpvOp op, SSA type, SSA id, std::vector<uint32_t> args)
{
	//TODO check for valid type, maybe.

	if (!isValidOp(op))
	{
		return;
	}

	if (type == 0)
	{
		//TODO complain
		return;
	}

	if (id == 0)
	{
		//TODO complain(?)
		return;
	}

	spvAsm->setOpForSSA(id, op);
	spvAsm->setSection(id, section);

	code.push_back(op);
	code.push_back(type);
	code.push_back(id);
	code.insert(code.end(), args.begin(), args.end());

}

void CodeSection::pushVar(SSA type, SSA id, StorageClass sc, SSA init)
{
	if (id == 0)
	{
		//TODO complain
		return;
	}

	varMeta.emplace(id, VarData{ id, type, sc });

	std::vector<uint32_t> args = { (uint32_t)sc };

	if (init != 0)
	{
		args.push_back(init);
	}

	pushTyped(OpVariable(init != 0), type, id, args );

}

void CodeSection::pushRaw(std::vector<uint32_t> args)
{
	code.insert(code.end(), args.begin(), args.end());

}

//Do NOT try to replace this with a memcpy
void CodeSection::pushStr(std::string str)
{
	code.reserve(code.size() + SpvStrLen(str));

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

void CodeSection::decorate(SSA target, Decoration d, std::vector<uint32_t> args)
{
	push(spirv::OpDecorate((uint32_t)args.size()), 0, { target, (uint32_t)d });
	pushRaw(args);
}

void CodeSection::decorateMember(SSA target, uint32_t member, Decoration d, std::vector<uint32_t> args)
{
	push(spirv::OpMemberDecorate((uint32_t)args.size()), 0, { target, member, (uint32_t)d });
	pushRaw(args);
}

SSA CodeSection::find(SpvOp op, std::vector<uint32_t> args)
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

bool CodeSection::findVarMeta(SSA id, ref<VarData> meta)
{
	auto found = varMeta.find(id);

	if (found == varMeta.end())
	{
		return false;
	}

	meta = found->second;

	return true;
}
