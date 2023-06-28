
#pragma once

#include <array>
#include <stdint.h>
#include <vector>

#include "langcore.h"
#include "syntax.h"

namespace caliburn
{
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
	namespace cllr
	{
		class Assembler;
		struct Instruction;

		using SSA = uint32_t;
		using InstructionVec = std::vector<sptr<Instruction>>;

		enum class Opcode : uint32_t
		{
			UNKNOWN,

			SHADER_STAGE,
			SHADER_STAGE_END,
			/*
			KERNEL,
			KERNEL_END,
			*/
			FUNCTION,
			VAR_FUNC_ARG,
			FUNCTION_END,

			VAR_LOCAL,
			VAR_GLOBAL,
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

			VALUE_CAST,
			VALUE_CONSTRUCT,
			CONSTRUCT_ARG,
			VALUE_DEREF,
			VALUE_EXPR,
			VALUE_EXPR_UNARY,
			VALUE_INVOKE_POS,
			VALUE_INVOKE_SIZE,
			VALUE_LIT_ARRAY,
			LIT_ARRAY_ELEM,
			VALUE_LIT_BOOL,
			VALUE_LIT_FP,
			VALUE_LIT_INT,
			VALUE_LIT_STR,
			VALUE_MEMBER,
			VALUE_NULL,
			VALUE_READ_VAR,
			VALUE_SUBARRAY,
			VALUE_ZERO,

			RETURN,
			RETURN_VALUE,
			DISCARD,

			CLLR_OP_COUNT

		};
		
		struct Instruction
		{
			SSA index = 0;
			Opcode op = Opcode::UNKNOWN;
			std::array<uint32_t, 3> operands = {};
			std::array<SSA, 3> refs = {};

			SSA outType = 0;
			sptr<Token> debugSym = nullptr;

		};

		struct CompilationUnit
		{
			Target target = Target::GPU;
			uptr<InstructionVec> code;

		};

	}

}
