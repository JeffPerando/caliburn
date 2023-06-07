
#include "cllrspirv.h"

using namespace caliburn;

uptr<std::vector<uint32_t>> cllr::SPIRVOutAssembler::translateCLLR(ref<cllr::Assembler> cllrAsm, ref<std::vector<sptr<cllr::Instruction>>> code)
{
	for (const auto& i : code)
	{
		auto fn = (impls[(uint32_t)i->op]);
		(*fn)(target, *i, cllrAsm, *this, *this->spirvAsm);

	}

	return spirvAsm->toShader();
}

spirv::SSA cllr::SPIRVOutAssembler::getOrCreateAlias(cllr::SSA ssa, spirv::SpvOp op)
{
	auto spvSSA = ssaAliases.find(ssa);

	if (spvSSA != ssaAliases.end())
	{
		return spvSSA->second;
	}

	auto nextSpvSSA = spirvAsm->createSSA(op);

	ssaAliases.emplace(ssa, nextSpvSSA);

	return nextSpvSSA;
}

//==========================================
//SPIR-V -> CLLR functions beyond this point
//==========================================

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpUnknown) {}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLabel)
{
	spv.main()->push(spirv::OpLabel(), {});
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

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeInt)
{
	spv.types()->push(spirv::OpTypeInt(), { i.operands[0], i.operands[1]});
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeFloat)
{
	spv.types()->push(spirv::OpTypeInt(), { i.operands[0] });
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeStruct)
{
	//auto ssa = spv.createSSA(spirv::OpTypeStruct());

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpEntryPoint)
{
	
}