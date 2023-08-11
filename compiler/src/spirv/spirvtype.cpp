
#include "spirv/spirvtype.h"

#include "spirv/cllrspirv.h"

using namespace caliburn::spirv;

bool TypeSection::findData(SSA id, ref<Type> out)
{
	auto found = ssaToType->find(id);

	if (found == ssaToType->end())
	{
		return false;
	}

	out = found->second;

	return true;
}

SSA TypeSection::findOrMake(SpvOp op, std::vector<uint32_t> args, SSA id)
{
	auto fid = types->find(Type{ op, 0, args });

	if (fid != types->end())
	{
		if (id != 0)
		{
			ssaToType->emplace(id, Type{ op, fid->second, args });
		}

		return fid->second;
	}

	if (id == 0)
	{
		id = spvAsm->createSSA();
		spvAsm->setOpForSSA(id, op);

	}

	ssaToType->emplace(id, Type{ op, id, args });
	types->emplace(Type{ op, id, args }, id);

	return id;
}
/*
void TypeSection::pushNew(SpvOp op, SSA id, std::vector<uint32_t> args)
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

	types->emplace(Type{ op, id, args }, id);

}
*/
void TypeSection::dump(ref<CodeSection> sec) const
{
	for (auto& [t, id] : *types)
	{
		sec.push(t.opcode, id, t.operands);

	}

}

SSA TypeSection::typeInt32()
{
	return findOrMake(OpTypeInt(), { 32, 1 });
}

SSA TypeSection::typeUInt32()
{
	return findOrMake(OpTypeInt(), { 32, 0 });
}

SSA TypeSection::typeFP32()
{
	return findOrMake(OpTypeFloat(), { 32 });
}

SSA TypeSection::typeBool()
{
	return findOrMake(OpTypeBool(), {});
}

SSA TypeSection::typeVec(uint32_t len, SSA inner)
{
	if (inner == 0)
	{
		inner = typeFP32();
	}

	return findOrMake(OpTypeVector(), { inner, len });
}

SSA TypeSection::typeArray(uint32_t len, SSA inner)
{
	return findOrMake(OpTypeArray(), { inner, len });
}

SSA TypeSection::typeStruct(std::vector<uint32_t> members, std::vector<BuiltIn> decs)
{
	auto id = findOrMake(OpTypeStruct((uint32_t)members.size()), members);

	if (!decs.empty())
	{
		for (uint32_t index = 0; index < decs.size(); ++index)
		{
			spvAsm->decs->decorateMember(id, index, Decoration::BuiltIn, { (uint32_t)decs[index] });

		}

	}

	return id;
}

SSA TypeSection::typePtr(SSA inner, StorageClass sc)
{
	return findOrMake(OpTypePointer(), { (uint32_t)sc, inner });
}

