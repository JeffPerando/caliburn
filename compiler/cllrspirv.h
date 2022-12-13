
#pragma once

#include <map>
#include <vector>

#include "cllr.h"
#include "spirvasm.h"

#define CLLR_SPIRV_IMPL(Name) spirv::SSA Name(cllr::Instruction i, spirv::CllrTranslator* t)

namespace caliburn
{
	namespace spirv
	{
		class CllrTranslator;

		//function pointer type for easier usage later
		using CllrImpl = spirv::SSA(cllr::Instruction i, spirv::CllrTranslator* t);

		namespace cllr_impl
		{
			CLLR_SPIRV_IMPL(OpUnknown);

			CLLR_SPIRV_IMPL(OpLabel);

			CLLR_SPIRV_IMPL(OpLoop);

			CLLR_SPIRV_IMPL(OpTypeInt);

			CLLR_SPIRV_IMPL(OpTypeFloat);

			CLLR_SPIRV_IMPL(OpTypeStruct);

		}
		
		class CllrTranslator
		{
		private:
			std::map<cllr::SSA, spirv::SSA> ssaAliases;

			CllrImpl* opImpls[(uint64_t)cllr::Opcode::OP_COUNT] = {};

		public:
			cllr::Assembler* const in;
			spirv::Assembler* const out;

			CllrTranslator(cllr::Assembler* inAsm, spirv::Assembler* outAsm) : in(inAsm), out(outAsm) {}

			virtual ~CllrTranslator() {}

			spirv::SSA getOrCreateAlias(cllr::SSA ssa, spirv::SpvOp op);

			void translate(std::vector<cllr::Instruction>* code);

		};

	}

}
