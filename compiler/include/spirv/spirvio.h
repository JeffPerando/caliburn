
#pragma once

#include <map>

#include "basic.h"
#include "spirv/spirvasm.h"

namespace caliburn
{
	namespace cllr
	{
		class SPIRVOutAssembler;
	}

	namespace spirv
	{
		class SpvIO
		{
			std::map<spirv::BuiltIn, spirv::SSA> builtinIOs;
			std::map<spirv::ExecutionModel, spirv::SSA> shaderOuts;

			const ptr<cllr::SPIRVOutAssembler> spvAsm;

			spirv::SSA typeInt32();

			spirv::SSA typeUInt32();

			spirv::SSA typeFP32();

			spirv::SSA typeVec(uint32_t len, spirv::SSA inner);

			spirv::SSA typeArray(uint32_t len, spirv::SSA inner);

			spirv::SSA typeStruct(std::vector<uint32_t> members, std::vector<spirv::BuiltIn> decs );

			spirv::SSA typePtr(spirv::SSA inner, spirv::StorageClass sc);

			spirv::SSA makeVar(spirv::SSA type, spirv::StorageClass sc);

		public:
			SpvIO(ptr<cllr::SPIRVOutAssembler> spv) : spvAsm(spv) {}
			
			spirv::SSA getBuiltinVar(spirv::BuiltIn builtin);

			spirv::SSA getOutputFor(spirv::ExecutionModel model, ref<spirv::SSA> outType, uint32_t loc = 0);

		};

	}

}
