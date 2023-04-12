
#pragma once

#include <map>
#include <vector>

#include "basic.h"
#include "cllr.h"
#include "spirvasm.h"

namespace caliburn
{
	namespace spirv
	{
		class CllrTranslator;

		//Function pointer type for easier usage later
		using CllrImpl = spirv::SSA(cllr::Instruction i, spirv::CllrTranslator* t);

		//Macro shorthand for implementation signature
		#define CLLR_SPIRV_IMPL(Name) spirv::SSA Name(cllr::Instruction i, spirv::CllrTranslator* t)

		namespace cllr_impl
		{
			CLLR_SPIRV_IMPL(OpUnknown);

			CLLR_SPIRV_IMPL(OpLabel);

			CLLR_SPIRV_IMPL(OpLoop);

			CLLR_SPIRV_IMPL(OpTypeInt);

			CLLR_SPIRV_IMPL(OpTypeFloat);

			CLLR_SPIRV_IMPL(OpTypeStruct);

			CLLR_SPIRV_IMPL(OpEntryPoint);

		}
		
		class CllrTranslator
		{
		private:
			std::map<cllr::SSA, spirv::SSA> ssaAliases;

			std::array<ptr<CllrImpl>, (uint64_t)cllr::Opcode::CLLR_OP_COUNT> opImpls = {};

		public:
			const ptr<cllr::Assembler> in;
			const ptr<spirv::Assembler> out;

			CllrTranslator(cllr::Assembler* inAsm, spirv::Assembler* outAsm) : in(inAsm), out(outAsm)
			{
				//here we go...

			}

			virtual ~CllrTranslator() {}

			spirv::SSA getOrCreateAlias(cllr::SSA ssa, spirv::SpvOp op);

			void translate(std::vector<cllr::Instruction>* code);

		};

	}

}
