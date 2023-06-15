
#pragma once

#include "basic.h"
#include "cllrasm.h"

namespace caliburn
{
	namespace cllr
	{
		class Validator
		{
			void validate(ref<InstructionVec> is);

			bool isType(cllr::Opcode op);

			bool isValue(cllr::Opcode op);

			bool isVar(cllr::Opcode op);

		};

	}

}
