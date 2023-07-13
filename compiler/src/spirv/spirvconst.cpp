
#include "spirv/spirvconst.h"

#include "spirv/cllrspirv.h"

using namespace caliburn::spirv;

SSA ConstSection::findOrMake(SSA t, uint32_t low, uint32_t high)
{
	auto key = Constant{ t, 0, low, high };
	auto c = consts->find(key);

	if (c != consts->end())
	{
		return c->second;
	}

	auto id = spvAsm->createSSA();

	key.id = id;

	consts->emplace(key, id);
	spvAsm->setSection(id, SSAKind::CONST);

	return id;
}

SSA ConstSection::findOrMakeComposite(SSA t, std::vector<uint32_t> data)
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
	spvAsm->setSection(id, SSAKind::CONST);

	return id;
}

SSA ConstSection::findOrMakeNullFor(SSA t)
{
	auto found = nulls->find(t);

	if (found != nulls->end())
	{
		return found->second;
	}

	auto nID = spvAsm->createSSA();

	nulls->emplace(t, nID);
	spvAsm->setSection(nID, SSAKind::CONST);

	return nID;
}

void ConstSection::dump(ref<CodeSection> sec) const
{
	for (auto& [typeID, nullID] : *nulls)
	{
		sec.pushTyped(OpConstantNull(), typeID, nullID, {});
	}

	for (auto& [data, id] : *consts)
	{
		if (data.type == OpTypeBool())
		{
			sec.pushTyped(data.lower == 0 ? OpConstantFalse() : OpConstantTrue(), data.type, id, {});
		}
		else
		{
			if (data.upper != 0)
			{
				sec.pushTyped(OpConstant(2), data.type, id, { data.lower, data.upper });

			}
			else
			{
				sec.pushTyped(OpConstant(1), data.type, id, { data.lower });

			}

		}

	}

	for (auto& [comp, id] : *composites)
	{
		sec.pushTyped(OpConstantComposite((uint32_t)comp.data.size()), comp.typeID, id, comp.data);
	}

}
