
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
		static auto constexpr MAX_OPS = 4;
		static auto constexpr MAX_REFS = 4;

		struct Assembler;
		
		struct Instruction;
		using InstructionVec = std::vector<Instruction>;

		using SSA = uint32_t;
		struct TypedSSA
		{
			SSA type = 0;
			SSA value = 0;

			TypedSSA() = default;
			TypedSSA(SSA t, SSA v) : type(t), value(v) {}

			bool operator==(in<TypedSSA> other) const
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
			VALUE_VEC_SWIZZLE,
			VALUE_ZERO,

			RETURN,
			RETURN_VALUE,
			DISCARD,

			CLLR_OP_COUNT

		};
		
		static const std::vector<std::string_view> OP_NAMES = {
			"Unknown",

			"Shader_Stage",
			"Shader_Stage_End",
			"Function",
			"Var_Func_Arg",
			"Function_End",

			"Var_Local",
			"Var_Global",
			"Var_Shader_In",
			"Var_Shader_Out",
			"Var_Descriptor",

			"Call",
			"Call_Arg",

			"Type_Void",
			"Type_Float",
			"Type_Int_Sign",
			"Type_Int_Unsign",
			"Type_Array",
			"Type_Vector",
			"Type_Matrix",

			"Type_Struct",
			"Struct_Member",
			"Struct_End",

			"Type_Bool",
			"Type_Ptr",
			"Type_Tuple",

			"Label",
			"Jump",
			"Jump_Cond",
			"Loop",

			"Assign",
			"Compare",

			"Value_Cast",
			"Value_Construct",
			"Construct_Arg",
			"Value_Deref",
			"Value_Expand",
			"Value_Expr",
			"Value_Expr_Unary",
			"Value_Int_To_Fp",
			"Value_Invoke_Pos",
			"Value_Invoke_Size",
			"Value_Lit_Array",
			"Lit_Array_Elem",
			"Value_Lit_Bool",
			"Value_Lit_Fp",
			"Value_Lit_Int",
			"Value_Lit_Str",
			"Value_Member",
			"Value_Null",
			"Value_Read_Var",
			"Value_Sign",
			"Value_Subarray",
			"Value_Unsign",
			"Value_Vec_Swizzle",
			"Value_Zero",

			"Return",
			"Return_Value",
			"Discard"
		};

		bool isType(Opcode op);

		bool isValue(Opcode op);

		bool isLValue(Opcode op);

		bool isVar(Opcode op);

		using OpArray = std::array<uint32_t, MAX_OPS>;
		using RefArray = std::array<SSA, MAX_REFS>;

		struct Instruction
		{
			SSA index = 0;
			Opcode op = Opcode::UNKNOWN;
			OpArray operands = {};
			RefArray refs = {};

			SSA outType = 0;
			sptr<Token> debugTkn = nullptr;

			Instruction() {}

			Instruction(Opcode op, OpArray ops = {}, RefArray rs = {}, SSA out = 0) :
				op(op), operands(ops), refs(rs), outType(out) {}

			Instruction(SSA id, Opcode op, OpArray ops = {}, RefArray rs = {}, SSA out = 0) :
				index(id), op(op), operands(ops), refs(rs), outType(out) {}

			Instruction(in<Instruction> old)
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

			std::string toStr() const
			{
				std::stringstream ss;

				if (debugTkn != nullptr)
				{
					ss << '@' << debugTkn->pos.toStr() << ": ";
				}

				if (index != 0)
				{
					ss << '#' << index << ": ";
				}

				ss << OP_NAMES[(uint32_t)op];

				ss << " [";
				for (size_t i = 0; i < MAX_OPS; ++i)
				{
					ss << operands[i];

					if ((i + 1) < MAX_OPS)
						ss << ", ";

				}
				ss << "] {";
				for (size_t i = 0; i < MAX_REFS; ++i)
				{
					ss << '#' << refs[i];

					if ((i + 1) < MAX_REFS)
						ss << ", ";

				}
				ss << '}';

				if (outType != 0)
					ss << " -> #" << outType;

				return ss.str();
			}

			bool operator<(in<Instruction> rhs) const
			{
				if (op >= rhs.op)
				{
					return false;
				}

				for (auto i = 0; i < MAX_OPS; ++i)
				{
					if (operands[i] >= rhs.operands[i])
					{
						return false;
					}
				}

				for (auto i = 0; i < MAX_REFS; ++i)
				{
					if (refs[i] >= rhs.refs[i])
					{
						return false;
					}
				}

				return true;
			}

			bool operator==(in<Instruction> other) const
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
			size_t operator()(in<Instruction> i) const
			{
				size_t hash = ((size_t)i.op & 0xFFFF);
				
				hash |= ((size_t)i.outType & 0xFFFF) << 16;

				size_t opHash = 0;

				for (size_t x = 0; x < MAX_OPS; ++x)
				{
					opHash ^= i.operands[x];
				}

				size_t refHash = 0;

				for (size_t x = 0; x < MAX_REFS; ++x)
				{
					refHash ^= i.refs[x];
				}

				hash |= (opHash & 0xFFFF) << 32;
				hash |= (refHash & 0xFFFF) << 48;
				
				return hash;
			}

		};

		struct CompilationUnit
		{
			HostTarget target = HostTarget::GPU;
			uptr<InstructionVec> code;

		};

	}

}
