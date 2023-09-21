
#pragma once

#include "cllr.h"

#include "basic.h"
#include "error.h"
#include "langcore.h"

namespace caliburn
{
	namespace cllr
	{
		template<typename OutImpl>
		using OutImpls = std::array<ptr<OutImpl>, (uint64_t)cllr::Opcode::CLLR_OP_COUNT>;

		/*
		This is the class for a CLLR output assembler; its job is to standardize the path from CLLR to an output target.

		It has one template type:
		* OutCode, which is a type which embodies the width of a word in the target; so 32-bit for SPIR-V for example.

		The compiler will make an OutAssembler based on the compilation requirements, then call translateCllr. Finally,
		it will take the resulting code and write it to a file, or RAM, or wherever. This is the final step in compiling
		Caliburn code.
		*/
		template<typename OutCode>
		struct OutAssembler
		{
			const uptr<ErrorHandler> errors;
			const HostTarget target;

			OutAssembler(sptr<const CompilerSettings> cs, HostTarget t) : target(t), errors(new_uptr<ErrorHandler>(CompileStage::OUT_EMIT, cs)) {}
			virtual ~OutAssembler() {}

			virtual uptr<std::vector<OutCode>> translateCLLR(in<cllr::Assembler> cllrAsm) = 0;

		};

	}

}
