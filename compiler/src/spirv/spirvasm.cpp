
#include "spirv/spirvasm.h"

#include "spirv/cllrspirv.h"

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
		spvAsm->setSection(id, section);

	}

	code.insert(code.end(), args.begin(), args.end());

}

void spirv::CodeSection::pushTyped(SpvOp op, SSA type, SSA id, std::vector<uint32_t> args)
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

void spirv::CodeSection::pushVar(SSA type, SSA id, StorageClass sc, SSA init)
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

	pushTyped(spirv::OpVariable(init != 0), type, id, args );

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

bool spirv::CodeSection::findVarMeta(SSA id, ref<VarData> meta)
{
	auto found = varMeta.find(id);

	if (found == varMeta.end())
	{
		return false;
	}

	meta = found->second;

	return true;
}

bool spirv::TypeSection::findData(SSA id, ref<Type> out)
{
	auto found = ssaToType->find(id);

	if (found == ssaToType->end())
	{
		return false;
	}

	out = found->second;

	return true;
}

spirv::SSA spirv::TypeSection::findOrMake(SpvOp op, std::vector<uint32_t> args, SSA id)
{
	auto fid = types->find(Type{ op, 0, args });

	if (fid != types->end())
	{
		if (id != 0)
		{
			//TODO edge case
			//just... alias them??? idk
		}

		return fid->second;
	}

	if (id == 0)
	{
		id = spvAsm->createSSA();
		spvAsm->setOpForSSA(op, id);

	}

	types->emplace(Type{ op, id, args }, id);

	return id;
}

void spirv::TypeSection::pushNew(SpvOp op, SSA id, std::vector<uint32_t> args)
{
	if (id == 0)
	{
		//TODO complain
		return;
	}

	if (types->find(Type{ op, 0, args }) != types->end())
	{
		//TODO complain
		return;
	}

	types->emplace(Type { op, id, args }, id);

}

void spirv::TypeSection::dump(ref<CodeSection> sec) const
{
	for (auto& [t, id] : *types)
	{
		sec.push(t.opcode, id, t.operands);

	}

}

spirv::SSA spirv::ConstSection::findOrMake(SSA t, uint32_t first, uint32_t second)
{
	auto key = Constant{ t, 0, first, second };
	auto c = consts->find(key);

	if (c != consts->end())
	{
		return c->second;
	}

	auto id = spvAsm->createSSA();

	key.id = id;

	consts->emplace(key, id);
	spvAsm->setSection(id, spirv::SSASection::CONST);

	return id;
}

spirv::SSA spirv::ConstSection::findOrMakeComposite(SSA t, std::vector<uint32_t> data)
{
	auto key = CompositeConst{ t, 0, data };

	auto c = composites->find(key);

	if (c != composites->end())
	{
		return c->second;
	}

	auto id = spvAsm->createSSA();

	key.id = id;

	composites->emplace(key, id);
	spvAsm->setSection(id, spirv::SSASection::CONST);

	return id;
}

spirv::SSA spirv::ConstSection::findOrMakeNullFor(SSA t)
{
	auto found = nulls->find(t);

	if (found != nulls->end())
	{
		return found->second;
	}

	auto nID = spvAsm->createSSA();

	nulls->emplace(t, nID);
	spvAsm->setSection(nID, spirv::SSASection::CONST);

}

void spirv::ConstSection::dump(ref<CodeSection> sec) const
{
	for (auto& [typeID, nullID] : *nulls)
	{
		sec.pushTyped(spirv::OpConstantNull(), typeID, nullID, {});
	}

	for (auto& [data, id] : *consts)
	{
		if (data.type == spirv::OpTypeBool())
		{
			sec.pushTyped(data.lower == 0 ? spirv::OpConstantFalse() : spirv::OpConstantTrue(), data.type, id, {});
		}
		else
		{
			if (data.upper != 0)
			{
				sec.pushTyped(spirv::OpConstant(2), data.type, id, { data.lower, data.upper });

			}
			else
			{
				sec.pushTyped(spirv::OpConstant(1), data.type, id, { data.lower });

			}

		}

	}

	for (auto& [comp, id] : *composites)
	{
		sec.pushTyped(spirv::OpConstantComposite((uint32_t)comp.data.size()), comp.typeID, id, comp.data);
	}

}
