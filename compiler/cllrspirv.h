
#pragma once

#include <map>
#include <vector>

#include "basic.h"
#include "cllrout.h"
#include "spirvasm.h"

namespace caliburn
{
	namespace cllr
	{
		//Function pointer type for easier usage later
		using SPIRVOutImpl = OutAsmImpl<spirv::Assembler>;

		//Macro shorthand for implementation signature
		#define CLLR_SPIRV_IMPL(Name) spirv::SSA Name(Target target, ref<const sptr<Instruction>> i, ref<spirv::Assembler> out)

		namespace spirv_impl
		{
			CLLR_SPIRV_IMPL(OpUnknown);

			CLLR_SPIRV_IMPL(OpLabel);

			CLLR_SPIRV_IMPL(OpLoop);

			CLLR_SPIRV_IMPL(OpTypeInt);

			CLLR_SPIRV_IMPL(OpTypeFloat);

			CLLR_SPIRV_IMPL(OpTypeStruct);

			CLLR_SPIRV_IMPL(OpEntryPoint);

		}
		
		class SPIRVOutAssembler : cllr::OutAssembler<uint32_t>
		{
		private:
			OutImpls<SPIRVOutImpl> impls;
			std::map<cllr::SSA, spirv::SSA> ssaAliases;

			spirv::Assembler spirvAsm;

		public:
			SPIRVOutAssembler() : OutAssembler(Target::GPU)
			{
				//here we go...

			}

			virtual ~SPIRVOutAssembler() {}

			spirv::SSA getOrCreateAlias(cllr::SSA ssa, spirv::SpvOp op);

			uptr<std::vector<spirv::SSA>> translateCLLR(ref<std::vector<sptr<cllr::Instruction>>> code) override;

		};

	}

}
