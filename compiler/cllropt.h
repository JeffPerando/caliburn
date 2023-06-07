
#pragma once

#include "cllrasm.h"
#include "langcore.h"

namespace caliburn
{
	namespace cllr
	{
		#define CLLR_OPTIMIZE_PASS(Name) void Name(ref<cllr::Assembler> codeAsm)

		void optimize(OptimizeLevel o, ref<cllr::Assembler> cllrAsm);

	}

}
