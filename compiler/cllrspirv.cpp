
#include "cllrspirv.h"

using namespace caliburn;

void spirv::CllrTranslator::translate(ref<std::vector<sptr<cllr::Instruction>>> code)
{
	for (const auto& i : code)
	{
		opImpls[(uint32_t)i->op](i, *this);
	}

}

spirv::SSA spirv::CllrTranslator::getOrCreateAlias(cllr::SSA ssa, spirv::SpvOp op)
{
	auto spvSSA = ssaAliases.find(ssa);

	if (spvSSA != ssaAliases.end())
	{
		return spvSSA->second;
	}

	auto nextSpvSSA = out->createSSA(op);

	ssaAliases.emplace(ssa, nextSpvSSA);

	return nextSpvSSA;
}

//==========================================
//SPIR-V -> CLLR functions beyond this point
//==========================================

CLLR_SPIRV_IMPL(spirv::cllr_impl::OpUnknown)
{
	return 0;
}

CLLR_SPIRV_IMPL(spirv::cllr_impl::OpLabel)
{
	return t.out->main()->push(spirv::OpLabel(), {});
}

/*
CLLR_SPIRV_IMPL(spirv::cllr_impl::OpLoop)
{
	spirv::SSA bodySSA = spv->nextSSA(spirv::OpLabel());

	//spv->main()->push(spirv::OpLoopMerge(0), { mergeSSA, contSSA, 0 }, false); //TODO base flag off optimize level
	//workaround for OpLoopMerge needing a branch op after
	spv->main()->push(spirv::OpBranch(), { bodySSA }, false);
	return spv->main()->push(spirv::OpLabel(), { bodySSA }, false);
}
*/

CLLR_SPIRV_IMPL(spirv::cllr_impl::OpTypeInt)
{
	return t.out->types()->push(spirv::OpTypeInt(), {i->operands[0], i->operands[1]});
}

CLLR_SPIRV_IMPL(spirv::cllr_impl::OpTypeFloat)
{
	return t.out->types()->push(spirv::OpTypeInt(), { i->operands[0] });
}

CLLR_SPIRV_IMPL(spirv::cllr_impl::OpTypeStruct)
{
	auto ssa = t.out->createSSA(spirv::OpTypeStruct());

	return 0;
}

CLLR_SPIRV_IMPL(spirv::cllr_impl::OpEntryPoint)
{
	

	return 0;
}