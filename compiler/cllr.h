
#pragma once

#include <stdint.h>

/*
Caliburn Low-Level Representation (CLLR [pronounced Caller]):

- Low-level bytecode for Caliburn code.
- Format: Tuples.
  - Fixed-width instructions for easier code navigation.
  - Instructions can define what the elements in the tuple do.
- Represents a shader pipeline.
- Flattened down to shader stages, functions, structs, and basic instructions.
- Not distributed; Only used internally by compiler.
*/

namespace caliburn
{
	namespace cllr
	{
		enum class Opcode : uint32_t
		{

		};

		struct Instruction
		{
			Opcode op;
			uint32_t operands[3];
		};

	}

}