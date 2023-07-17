
#pragma once

#include <vector>

#include "cllr.h"

namespace caliburn
{
	namespace cllr
	{
		class Finder
		{
			std::vector<Opcode> opcodes;
			std::vector<uint32_t> op_args;
			SSA refID = 0;
			SSA outType = 0;

			uint32_t offset = 0;
			uint32_t limit = UINT32_MAX;

		public:
			Finder() = default;
			virtual ~Finder() {}

			ptr<Finder> ops(std::vector<Opcode> ops);
			ptr<Finder> operands(std::array<uint32_t, 3> operands);
			ptr<Finder> references(SSA ref);
			ptr<Finder> outputs(SSA type);
			ptr<Finder> setOffset(size_t off);
			ptr<Finder> setLimit(uint32_t max);

			InstructionVec find(ref<const InstructionVec> code);

		};

	}

}