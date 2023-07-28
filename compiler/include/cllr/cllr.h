
#pragma once

#include <array>
#include <stdint.h>
#include <vector>

#include "langcore.h"
#include "syntax.h"

namespace caliburn
{
	struct RealType;

	/*
	Caliburn Low-Level Representation (CLLR [pronounced Caller]):

	- Low-level bytecode for Caliburn code.
	- Format: Tuples.
	  - Fixed-width instructions for easier code navigation.
	  - Instructions define what the elements in the tuple do.
	- Represents a single compilation unit.
	- Flattened down to shader stages, functions, structs, and basic instructions.
	- Not distributed; Only used internally by compiler.
	*/
	namespace cllr
	{
		struct Assembler;
		
		struct Instruction;
		using InstructionVec = std::vector<sptr<Instruction>>;

		using SSA = uint32_t;
		struct TypedSSA
		{
			//sptr<RealType> typePtr = nullptr;
			SSA type = 0;
			SSA value = 0;

			TypedSSA() = default;
			TypedSSA(SSA t, SSA v) : type(t), value(v) {}

			bool operator==(const TypedSSA& other) const
			{
				return type == other.type && value == other.value;
			}

		};

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
			TYPE_INT_SIGN,
			TYPE_INT_UNSIGN,
			TYPE_ARRAY,
			TYPE_VECTOR,
			TYPE_MATRIX,

			TYPE_STRUCT,
			STRUCT_MEMBER,
			STRUCT_END,

			TYPE_BOOL,
			TYPE_PTR,
			TYPE_TUPLE,
			//TYPE_STRING,

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
			VALUE_EXPAND,
			VALUE_EXPR,
			VALUE_EXPR_UNARY,
			VALUE_INT_TO_FP,
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
			VALUE_SIGN,
			VALUE_SUBARRAY,
			VALUE_UNSIGN,
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
			sptr<Token> debugTkn = nullptr;

			Instruction() {}

			Instruction(Opcode op, std::array<uint32_t, 3> ops = {}, std::array<SSA, 3> rs = {}, SSA out = 0) :
				op(op), operands(ops), refs(rs), outType(out) {}

			Instruction(SSA id, Opcode op, std::array<uint32_t, 3> ops = {}, std::array<SSA, 3> rs = {}, SSA out = 0) :
				index(id), op(op), operands(ops), refs(rs), outType(out) {}

			Instruction(const Instruction& old)
			{
				index = old.index;
				op = old.op;
				operands = old.operands;
				refs = old.refs;
				outType = old.outType;
				debugTkn = old.debugTkn;
			}

			Instruction& debug(sptr<Token> tkn)
			{
				debugTkn = tkn;
				return *this;
			}

			bool operator<(ref<const Instruction> rhs) const
			{
				if (op >= rhs.op)
				{
					return false;
				}

				for (uint32_t i = 0; i < 3; ++i)
				{
					if (operands[i] >= rhs.operands[i])
					{
						return false;
					}

					if (refs[i] >= rhs.refs[i])
					{
						return false;
					}

				}

				return true;
			}

			bool operator==(ref<const Instruction> other) const
			{
				if (index != other.index)
				{
					return false;
				}

				if (op != other.op)
				{
					return false;
				}

				if (operands != other.operands)
				{
					return false;
				}

				if (refs != other.refs)
				{
					return false;
				}

				if (outType != other.outType)
				{
					return false;
				}

				return true;
			}

		};

		struct InstructionHash
		{
			size_t operator()(const ref<Instruction> i) const
			{
				size_t hash = ((size_t)i.op & 0xFFFF) << 48;

				hash |= size_t(i.operands[0] & 0xFF) << 40;
				hash |= size_t(i.operands[1] & 0xFF) << 32;
				hash |= size_t(i.operands[2] & 0xFF) << 24;

				hash |= size_t(i.refs[0] & 0xFF) << 16;
				hash |= size_t(i.refs[0] & 0xFF) << 8;
				hash |= size_t(i.refs[0] & 0xFF);

				return hash;
			}

		};

		struct CompilationUnit
		{
			Target target = Target::GPU;
			uptr<InstructionVec> code;

		};

	}

}
