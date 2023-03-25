
#pragma once

#include <array>
#include <stdint.h>

/*
Caliburn Low-Level Representation (CLLR [pronounced Caller]):

- Low-level bytecode for Caliburn code.
- Format: Tuples.
  - Fixed-width instructions for easier code navigation.
  - Instructions can define what the elements in the tuple do.
- Represents a single compilation unit.
- Flattened down to shader stages, functions, structs, and basic instructions.
- Not distributed; Only used internally by compiler.
*/

namespace caliburn
{
	namespace cllr
	{
		class Assembler;

		using SSA = uint32_t;

		enum class Target : uint32_t
		{
			CPU, GPU
		};

		enum class Opcode : uint32_t
		{
			UNKNOWN,

			ENTRY_POINT,
			
			SHADER,
			SHADER_STAGE,
			DESCRIPTOR,
			SHADER_END,

			FUNCTION,
			FUNCTION_KERNEL,
			FUNCTION_END,

			VAR_LOCAL,
			VAR_GLOBAL,
			VAR_FUNC_ARG,
			VAR_SHADER_IN,
			VAR_SHADER_OUT,

			CALL,
			DISPATCH,
			CALL_ARG,

			TYPE_FLOAT,
			TYPE_INT,
			TYPE_STRUCT,
			TYPE_ARRAY,
			TYPE_VECTOR,
			TYPE_MATRIX,

			TYPE_BOOL,
			TYPE_PTR,
			TYPE_TUPLE,
			TYPE_STRING,

			STRUCT_MEMBER,
			STRUCT_END,

			LABEL,
			JUMP,
			JUMP_COND,
			LOOP,

			ASSIGN,
			ASSIGN_ARRAY,
			BINOP,
			COMPARE,

			VALUE_ARRAY_LIT,
			VALUE_CAST,
			VALUE_DEREF,
			VALUE_DESCRIPTOR,
			VALUE_INIT,
			VALUE_LITERAL,
			VALUE_MEMBER,
			VALUE_NULL,
			VALUE_SUBARRAY,
			VALUE_VARIABLE,
			
			RETURN,
			RETURN_VALUE,

			OP_COUNT

		};

		struct Instruction
		{
			SSA index = 0;
			Opcode op = Opcode::UNKNOWN;
			std::array<uint32_t, 3> operands {0, 0, 0};

		};

		struct CompilationUnit
		{
			Target target = Target::GPU;
			std::vector<Instruction> code;

		};

	}

}