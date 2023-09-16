
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
			sptr<const CompilerSettings> settings;

			Optimizer(sptr<const CompilerSettings> cs) : settings(cs) {}

			void optimize(out<cllr::Assembler> cllrAsm);

		};

	}

}
