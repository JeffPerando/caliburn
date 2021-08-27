
#include "allasm.h"
#include "typevoid.h"

using namespace caliburn;

uint32_t TypeVoid::getSizeBytes() const
{
	return 0;
}

uint32_t TypeVoid::getAlignBytes() const
{
	return 0;
}

//idk why anyone would call this but w/e
//it's only here to make a new clone of a generic
CompiledType* TypeVoid::clone()
{
	return new TypeVoid();
}

TypeCompat TypeVoid::isCompatible(Operator op, CompiledType* rType) const
{
	if (rType == nullptr)
	{
		return TypeCompat::INCOMPATIBLE_OP;
	}

	TypeCompat::INCOMPATIBLE_TYPE;
}

uint32_t TypeVoid::typeDeclSpirV(SpirVAssembler* codeAsm)
{
	if (ssa != 0)
	{
		return ssa;
	}

	ssa = codeAsm->newAssign();
	codeAsm->pushAll({spirv::OpTypeVoid(), ssa});
	return ssa;
}

uint32_t TypeVoid::mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const
{
	return 0;
}

uint32_t TypeVoid::mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const
{
	return 0;
}
