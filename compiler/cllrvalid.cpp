
#include <algorithm>

#include "cllrvalid.h"

using namespace caliburn;

void cllr::Validator::validate(ref<InstructionVec> is)
{

}

bool cllr::Validator::isType(cllr::Opcode op)
{
	std::vector<cllr::Opcode> ops = {
		Opcode::TYPE_VOID,
		Opcode::TYPE_FLOAT,
		Opcode::TYPE_INT,
		Opcode::TYPE_ARRAY,
		Opcode::TYPE_VECTOR,
		Opcode::TYPE_MATRIX,
		Opcode::TYPE_STRUCT,
		Opcode::TYPE_BOOL,
		Opcode::TYPE_PTR,
		Opcode::TYPE_TUPLE,
		Opcode::TYPE_STRING
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}

bool cllr::Validator::isValue(cllr::Opcode op)
{
    std::vector<cllr::Opcode> ops = {
		Opcode::CALL,
		Opcode::COMPARE,
		Opcode::VALUE_CAST,
		Opcode::VALUE_DEREF,
		Opcode::VALUE_EXPR,
		Opcode::VALUE_EXPR_UNARY,
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
		Opcode::VALUE_SUBARRAY,
		Opcode::VALUE_ZERO
	};

    return std::binary_search(ops.begin(), ops.end(), op);
}

bool cllr::Validator::isVar(cllr::Opcode op)
{
	std::vector<cllr::Opcode> ops = {
		Opcode::VAR_LOCAL,
		Opcode::VAR_GLOBAL,
		Opcode::VAR_FUNC_ARG,
		Opcode::VAR_SHADER_IN,
		Opcode::VAR_SHADER_OUT,
		Opcode::VAR_DESCRIPTOR
	};

	return std::binary_search(ops.begin(), ops.end(), op);
}
