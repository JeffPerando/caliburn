
#include "allasm.h"
#include "typebool.h"

using namespace caliburn;

TypeCompat TypeBool::isCompatible(Operator op, CompiledType* rType) const
{
	if (!rType)
	{
		if (op == Operator::BOOL_NOT)
		{
			return TypeCompat::COMPATIBLE;
		}

		return TypeCompat::INCOMPATIBLE_OP;
	}
	
	if (op == Operator::ADD || op == Operator::MUL)
	{
		if (rType->category == TypeCategory::PRIMITIVE)
		{
			return TypeCompat::COMPATIBLE;
		}

		return TypeCompat::INCOMPATIBLE_TYPE;
	}

	if (rType->category != TypeCategory::BOOLEAN)
	{
		return TypeCompat::INCOMPATIBLE_TYPE;
	}

	switch (op)
	{
	case Operator::COMP_EQ:
	case Operator::COMP_NEQ:
	case Operator::COND_AND:
	case Operator::COND_OR: return TypeCompat::COMPATIBLE;
	}

	return TypeCompat::INCOMPATIBLE_OP;
}

uint32_t TypeBool::typeDeclSpirV(SpirVAssembler* codeAsm)
{
	if (ssa)
	{
		return ssa;
	}

	ssa = codeAsm->newAssign();
	codeAsm->pushAll({spirv::OpTypeBool(), ssa});
	return ssa;
}

uint32_t TypeBool::mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const
{
	uint32_t lhs = lvalueSSA;

	if (op == Operator::ADD || op == Operator::MUL)
	{

	}

	return 0;
}

uint32_t TypeBool::mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const
{
	return 0;
}
