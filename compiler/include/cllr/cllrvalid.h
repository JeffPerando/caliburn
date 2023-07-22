
#pragma once

#include "basic.h"

#include "cllrasm.h"
#include "error.h"

namespace caliburn
{
	namespace cllr
	{
		struct Validator
		{
			const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::CLLR_VALIDATION);
			const CompilerSettings settings;

			Validator(ref<const CompilerSettings> cs) : settings(cs) {}

			bool validate(ref<const InstructionVec> is);

			bool isType(cllr::Opcode op);

			bool isValue(cllr::Opcode op);

			bool isLValue(cllr::Opcode op);

			bool isVar(cllr::Opcode op);

		};

	}

}
