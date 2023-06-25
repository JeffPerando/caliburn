
#pragma once

#include <initializer_list>
#include <vector>

#include "cbir.h"

namespace caliburn
{
	namespace cbir
	{
		class CBIRAssembler
		{
		private:
			std::vector<uint32_t> code;
		public:
			void push(Opcode op, std::initializer_list<uint32_t> operands);



		};

	}

}
