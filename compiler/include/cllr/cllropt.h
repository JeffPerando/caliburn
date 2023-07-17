
#pragma once

#include "cllrasm.h"
#include "langcore.h"

namespace caliburn
{
	namespace cllr
	{
		#define CLLR_OPTIMIZE_PASS(Name) void Name(ref<cllr::Assembler> codeAsm)

		struct Optimizer
		{
			const CompilerSettings settings;

			Optimizer(ref<const CompilerSettings> cs) : settings(cs) {}

			void optimize(ref<cllr::Assembler> cllrAsm);

		};

	}

}
