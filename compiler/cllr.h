
#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "langcore.h"

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
			UNKNOWN,
			
			SHADER,
			SHADER_STAGE,
			INPUT,
			OUTPUT,
			DESCRIPTOR,
			SHADER_END,

			FUNCTION,
			FUNCTION_ARG,
			FUNCTION_END,

			VARIABLE,

			CALL,
			CALL_ARG,

			TYPE_STRUCT,
			TYPE_FLOAT,
			TYPE_INT,
			TYPE_BOOL,
			TYPE_PTR,
			TYPE_ARRAY,
			TYPE_VECTOR,
			TYPE_MATRIX,

			STRUCT_MEMBER,
			STRUCT_END,

			LABEL,
			JUMP,
			JUMP_COND,

			ASSIGN,
			ASSIGN_ARRAY,
			BINOP,
			COMPARE,

			VALUE_ARGUMENT,
			VALUE_ARRAY,
			VALUE_CONVERT,
			VALUE_DEREF,
			VALUE_DESCRIPTOR,
			VALUE_INIT,
			VALUE_LITERAL,
			VALUE_MEMBER,
			VALUE_NULL,
			VALUE_SUBARRAY,
			VALUE_VARIABLE,
			
			RETURN,

		};

		struct Instruction
		{
			Opcode op;
			uint32_t operands[3] {0};
		};

		struct CLLRPipeline
		{
			std::string name;
			std::vector<std::string> strs;
			std::vector<Instruction> code;

		};



	}

}