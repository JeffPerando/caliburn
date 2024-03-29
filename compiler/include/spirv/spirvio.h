
#pragma once

#include <map>

#include "basic.h"
#include "spirv/spirvasm.h"

namespace caliburn
{
	namespace cllr
	{
		struct SPIRVOutAssembler;
	}

	namespace spirv
	{
		struct BuiltinVar
		{
			std::vector<BuiltIn> builtins;
			StorageClass kind;
			SSA typeID;
			std::vector<ExecutionModel> execs;
			std::vector<Capability> caps;
		};

		/*
		Maintains the shader inputs and outputs, as well as builtin variables
		*/
		class SpvIO
		{
			std::map<spirv::BuiltIn, spirv::SSA> builtinIOs;
			std::map<spirv::ExecutionModel, spirv::SSA> shaderOuts;

			const ptr<cllr::SPIRVOutAssembler> spvAsm;

			spirv::SSA makeVar(spirv::SSA type, spirv::StorageClass sc);

			spirv::SSA makeInVar(spirv::SSA type);

			spirv::SSA makeOutVar(spirv::SSA type);

		public:
			SpvIO(ptr<cllr::SPIRVOutAssembler> spv) : spvAsm(spv) {}
			
			spirv::SSA getBuiltinVar(spirv::ExecutionModel model, spirv::BuiltIn builtin);

			spirv::SSA getOutputFor(spirv::ExecutionModel model, out<spirv::SSA> outType, uint32_t loc = 0);

		};

	}

}
