
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
	  - Instructions define what the elements in the tuple do.
	- Represents a single compilation unit.
	- Flattened down to shader stages, functions, structs, and basic instructions.
	- Not distributed; Only used internally by compiler.
	*/
	namespace cllr
	{
		static auto constexpr MAX_OPS = 4;
		static auto constexpr MAX_REFS = 4;

		struct LowType;
		struct Assembler;

		struct Instruction;

		using SSA = uint32_t;

		struct TypedSSA
		{
			sptr<LowType> type = nullptr;
			SSA value = 0;

			TypedSSA() = default;
			TypedSSA(sptr<LowType> t, SSA v) : type(t), value(v) {}

			bool operator==(in<TypedSSA> other) const
			{
				return type == other.type && value == other.value;
			}

		};

		struct Loop
		{
			SSA start, end;
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
			TYPE_VOID,
			TYPE_FLOAT,
			TYPE_INT_SIGN,
			TYPE_INT_UNSIGN,
			TYPE_ARRAY,
			TYPE_VECTOR,
			TYPE_MATRIX,
			TYPE_TEXTURE,

			TYPE_STRUCT,
			STRUCT_MEMBER,
			STRUCT_END,

			TYPE_BOOL,
			TYPE_PTR,
			TYPE_TUPLE,
			//TYPE_STRING,

			SCOPE_BEGIN,
			SCOPE_END,

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
			VALUE_SAMPLE,
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

		static const std::map<Opcode, std::string_view> OP_NAMES = {
			{Opcode::UNKNOWN, "Unknown"},

			{Opcode::SHADER_STAGE, "Shader_Stage"},
			{Opcode::SHADER_STAGE_END, "Shader_Stage_End"},
			{Opcode::FUNCTION, "Function"},
			{Opcode::VAR_FUNC_ARG, "Var_Func_Arg"},
			{Opcode::FUNCTION_END, "Function_End"},

			{Opcode::VAR_LOCAL, "Var_Local"},
			{Opcode::VAR_GLOBAL, "Var_Global"},
			{Opcode::VAR_SHADER_IN, "Var_Shader_In"},
			{Opcode::VAR_SHADER_OUT, "Var_Shader_Out"},
			{Opcode::VAR_DESCRIPTOR, "Var_Descriptor"},

			{Opcode::CALL, "Call"},
			{Opcode::CALL_ARG, "Call_Arg"},

			{Opcode::TYPE_VOID, "Type_Void"},
			{Opcode::TYPE_FLOAT, "Type_Float"},
			{Opcode::TYPE_INT_SIGN, "Type_Int_Sign"},
			{Opcode::TYPE_INT_UNSIGN, "Type_Int_Unsign"},
			{Opcode::TYPE_ARRAY, "Type_Array"},
			{Opcode::TYPE_VECTOR, "Type_Vector"},
			{Opcode::TYPE_MATRIX, "Type_Matrix"},
			{Opcode::TYPE_TEXTURE, "Type_Texture"},

			{Opcode::TYPE_STRUCT, "Type_Struct"},
			{Opcode::STRUCT_MEMBER, "Struct_Member"},
			{Opcode::SCOPE_END, "Struct_End"},

			{Opcode::TYPE_BOOL, "Type_Bool"},
			{Opcode::TYPE_PTR, "Type_Ptr"},
			{Opcode::TYPE_TUPLE, "Type_Tuple"},

			{Opcode::SCOPE_BEGIN, "Scope_Start"},
			{Opcode::SCOPE_END, "Scope_End"},
			{Opcode::LABEL, "Label"},
			{Opcode::JUMP, "Jump"},
			{Opcode::JUMP_COND, "Jump_Cond"},
			{Opcode::LOOP, "Loop"},

			{Opcode::ASSIGN, "Assign"},
			{Opcode::COMPARE, "Compare"},

			{Opcode::VALUE_CAST, "Value_Cast"},
			{Opcode::VALUE_CONSTRUCT, "Value_Construct"},
			{Opcode::CONSTRUCT_ARG, "Construct_Arg"},
			{Opcode::VALUE_DEREF, "Value_Deref"},
			{Opcode::VALUE_EXPAND, "Value_Expand"},
			{Opcode::VALUE_EXPR, "Value_Expr"},
			{Opcode::VALUE_EXPR_UNARY, "Value_Expr_Unary"},
			{Opcode::VALUE_INT_TO_FP, "Value_Int_To_Fp"},
			{Opcode::VALUE_INVOKE_POS, "Value_Invoke_Pos"},
			{Opcode::VALUE_INVOKE_SIZE, "Value_Invoke_Size"},
			{Opcode::VALUE_LIT_ARRAY, "Value_Lit_Array"},
			{Opcode::LIT_ARRAY_ELEM, "Lit_Array_Elem"},
			{Opcode::VALUE_LIT_BOOL, "Value_Lit_Bool"},
			{Opcode::VALUE_LIT_FP, "Value_Lit_Fp"},
			{Opcode::VALUE_LIT_INT, "Value_Lit_Int"},
			{Opcode::VALUE_LIT_STR, "Value_Lit_Str"},
			{Opcode::VALUE_MEMBER, "Value_Member"},
			{Opcode::VALUE_NULL, "Value_Null"},
			{Opcode::VALUE_READ_VAR, "Value_Read_Var"},
			{Opcode::VALUE_SAMPLE, "Value_Sample"},
			{Opcode::VALUE_SIGN, "Value_Sign"},
			{Opcode::VALUE_SUBARRAY, "Value_Subarray"},
			{Opcode::VALUE_UNSIGN, "Value_Unsign"},
			{Opcode::VALUE_VEC_SWIZZLE, "Value_Vec_Swizzle"},
			{Opcode::VALUE_ZERO, "Value_Zero"},

			{Opcode::RETURN, "Return"},
			{Opcode::RETURN_VALUE, "Return_Value"},
			{Opcode::DISCARD, "Discard"},
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
			Token debugTkn;

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

			Instruction& debug(in<Token> tkn)
			{
				debugTkn = tkn;
				return *this;
			}

			std::string toStr() const
			{
				std::stringstream ss;

				if (debugTkn.exists())
				{
					ss << '@' << debugTkn.pos.toStr() << ": ";
				}

				if (index != 0)
				{
					ss << '#' << index << ": ";
				}

				ss << OP_NAMES.at(op);

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
			uptr<std::vector<Instruction>> code;

		};

	}

}
