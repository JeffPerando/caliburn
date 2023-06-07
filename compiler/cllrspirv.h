
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
		class SPIRVOutAssembler;

		//Function pointer type for easier usage later
		using SPIRVOutFn = void(Target target, ref<cllr::Instruction> i, ref<cllr::Assembler> in, ref<SPIRVOutAssembler> out, ref<spirv::Assembler> spv);

		//Macro shorthand for implementation signature
		#define CLLR_SPIRV_IMPL(Name) void Name(Target target, ref<Instruction> i, ref<cllr::Assembler> in, ref<SPIRVOutAssembler> out, ref<spirv::Assembler> spv)

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
			OutImpls<SPIRVOutFn> impls = {};
			std::map<cllr::SSA, spirv::SSA> ssaAliases;

			uptr<spirv::Assembler> spirvAsm = nullptr;

		public:
			SPIRVOutAssembler() : OutAssembler(Target::GPU)
			{
				spirvAsm = std::make_unique<spirv::Assembler>();

				//here we go...

			}

			virtual ~SPIRVOutAssembler() {}

			spirv::SSA getOrCreateAlias(cllr::SSA ssa, spirv::SpvOp op);

			uptr<std::vector<spirv::SSA>> translateCLLR(ref<cllr::Assembler> cllrAsm, ref<std::vector<sptr<cllr::Instruction>>> code) override;

		};

	}

}
