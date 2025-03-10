
#include "cllr/cllr.h"

#include <algorithm>

using namespace caliburn;

bool cllr::isType(Opcode op)
{
	const auto ops = {
		Opcode::TYPE_VOID,
		Opcode::TYPE_FLOAT,
		Opcode::TYPE_INT_SIGN,
		Opcode::TYPE_INT_UNSIGN,
		Opcode::TYPE_ARRAY,
		Opcode::TYPE_VECTOR,
		Opcode::TYPE_MATRIX,
		Opcode::TYPE_TEXTURE,
		Opcode::TYPE_STRUCT,
		Opcode::TYPE_BOOL,
		Opcode::TYPE_PTR,
		Opcode::TYPE_TUPLE,
		//Opcode::TYPE_STRING
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

bool cllr::isValue(Opcode op)
{
	const auto ops = {
		Opcode::CALL,
		Opcode::COMPARE,
		Opcode::VALUE_CAST,
		Opcode::VALUE_CONSTRUCT,
		Opcode::VALUE_DEREF,
		Opcode::VALUE_EXPAND,
		Opcode::VALUE_EXPR,
		Opcode::VALUE_EXPR_UNARY,
		Opcode::VALUE_INT_TO_FP,
		Opcode::VALUE_INVOKE_POS,
		Opcode::VALUE_INVOKE_SIZE,
		Opcode::VALUE_LIT_ARRAY,
		Opcode::VALUE_LIT_BOOL,
		Opcode::VALUE_LIT_FP,
		Opcode::VALUE_LIT_INT,
		Opcode::VALUE_LIT_STR,
		Opcode::VALUE_MEMBER,
		Opcode::VALUE_NULL,
		Opcode::VALUE_READ_VAR,
		Opcode::VALUE_SAMPLE,
		Opcode::VALUE_SIGN,
		Opcode::VALUE_SUBARRAY,
		Opcode::VALUE_UNSIGN,
		Opcode::VALUE_VEC_SWIZZLE,
		Opcode::VALUE_ZERO
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

bool cllr::isLValue(Opcode op)
{
	const auto ops = {
		Opcode::VALUE_DEREF,
		Opcode::VALUE_MEMBER,
		Opcode::VALUE_READ_VAR,
		Opcode::VALUE_SUBARRAY
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

bool cllr::isVar(Opcode op)
{
	const auto ops = {
		Opcode::VAR_LOCAL,
		Opcode::VAR_GLOBAL,
		Opcode::VAR_FUNC_ARG,
		Opcode::VAR_SHADER_IN,
		Opcode::VAR_SHADER_OUT,
		Opcode::VAR_DESCRIPTOR
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}
