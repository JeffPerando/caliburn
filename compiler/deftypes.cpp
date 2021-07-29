
#include "assembler.h"
#include "deftypes.h"

using namespace caliburn;

bool IntType::isCompatible(Operator op, CompiledType* rType) const
{
	if (rType->category == TypeCategory::PRIMITIVE)
	{
		return op != Operator::APPEND;
	}
	return false;
}

uint32_t IntType::typeDeclSpirV(SpirVAssembler* codeAsm) const
{
	auto ssa = codeAsm->newAssign();
	codeAsm->pushAll({ spirv::OpTypeInt(), ssa, size, (uint32_t)hasA(TypeAttrib::TA_SIGNED) });
	return ssa;
}

uint32_t IntType::mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA)
{
	if (rType->hasA(TA_FLOAT))
	{
		//TODO convert current type to equivalent floating point
		return 0;
	}
	SpvOp opcode = spirv::OpNop();
	/*
	ADD, SUB, MUL, DIV,
	MOD, POW, BIT_AND, BIT_OR,
	BIT_XOR, APPEND, INTDIV,
	BOOL_NOT, BIT_NOT, NEGATE,
	*/
	switch (op)
	{
	case Operator::ADD: opcode = spirv::OpIAdd(); break;
	case Operator::SUB: opcode = spirv::OpISub(); break;
	case Operator::MUL: opcode = spirv::OpIMul(); break;
	case Operator::DIV:
		if (!rType->hasA(TA_SIGNED) && !hasA(TA_SIGNED))
			opcode = spirv::OpUDiv();
		else opcode = spirv::OpSDiv();
		break;
	case Operator::MOD:
		if (!rType->hasA(TA_SIGNED) && !hasA(TA_SIGNED))
			opcode = spirv::OpUMod();
		else opcode = spirv::OpSMod();
		break;
	case Operator::BIT_AND: opcode = spirv::OpBitwiseAnd(); break;
	case Operator::BIT_OR: opcode = spirv::OpBitwiseOr(); break;
	case Operator::BIT_XOR: opcode = spirv::OpBitwiseXor(); break;
	}
	/*
	if (opcode == spirv::OpNop())
	{
		if (op == Operator::ARRAY_ACCESS)
		{
			uint32_t shifted = codeAsm->newAssign();
			codeAsm->pushAll({
				spirv::OpShiftRightLogical(), shifted, lvalueSSA, rvalueSSA
				});

		}
	}
	*/
}
