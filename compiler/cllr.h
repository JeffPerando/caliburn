
#pragma once

#include <array>
#include <stdint.h>
#include <vector>

#include "langcore.h"

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
		struct Instruction;

		using SSA = uint32_t;
		using InstructionVec = std::vector<sptr<Instruction>>;

		enum class Opcode : uint32_t
		{
			UNKNOWN,

			KERNEL,
			KERNEL_END,

			FUNCTION,
			FUNCTION_END,

			VAR_LOCAL,
			VAR_GLOBAL,
			VAR_FUNC_ARG,
			VAR_SHADER_IN,
			VAR_SHADER_OUT,
			VAR_DESCRIPTOR,

			CALL,
			//DISPATCH,
			CALL_ARG,

			//These should match the TypeCategory in langcore.h
			//enum class TypeCategory { VOID, FLOAT, INT, VECTOR, MATRIX, ARRAY, STRUCT, BOOLEAN, POINTER, TUPLE, STRING };
			TYPE_VOID,
			TYPE_FLOAT,
			TYPE_INT,
			TYPE_ARRAY,
			TYPE_VECTOR,
			TYPE_MATRIX,

			TYPE_STRUCT,
			STRUCT_MEMBER,
			STRUCT_END,

			TYPE_BOOL,
			TYPE_PTR,
			TYPE_TUPLE,
			TYPE_STRING,

			LABEL,
			JUMP,
			JUMP_COND,
			LOOP,

			ASSIGN,
			COMPARE,

			VALUE_ARRAY_LIT,
			VALUE_BOOL_LIT,
			VALUE_CAST,
			VALUE_DEREF,
			VALUE_DESCRIPTOR,
			VALUE_EXPR,
			VALUE_EXPR_UNARY,
			VALUE_FP_LIT,
			//VALUE_FN_CALL,
			VALUE_INIT,
			VALUE_INT_LIT,
			VALUE_INVOKE_POS,
			VALUE_INVOKE_SIZE,
			VALUE_MEMBER,
			VALUE_NULL,
			VALUE_STR_LIT,
			VALUE_SUBARRAY,
			VALUE_VARIABLE,

			RETURN,
			RETURN_VALUE,
			DISCARD,

			CLLR_OP_COUNT

		};
		
		/*
		CLLR instructions are now typed. Most will be type VOID.
		*/
		struct IType
		{
			TypeCategory cat = TypeCategory::VOID;
			uint32_t len = 1;
			std::array<uint32_t, 2> misc = { 0, 0 };

		};

		struct Instruction
		{
			SSA index = 0;
			Opcode op = Opcode::UNKNOWN;
			IType type;
			std::array<uint32_t, 3> operands = {};
			std::array<SSA, 3> refs = {};

		};

		struct CompilationUnit
		{
			Target target = Target::GPU;
			uptr<InstructionVec> code;

		};

	}

}
