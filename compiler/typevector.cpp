
#include "assembler.h"
#include "typevector.h"

using namespace caliburn;

uint32_t VectorType::getSizeBytes() const
{
	return vecElements * generics[0]->getSizeBytes();
}

uint32_t VectorType::getAlignBytes() const
{
	return generics[0]->getAlignBytes();
}

CompiledType* VectorType::clone()
{
	return new VectorType(vecElements, generics[0]);
}

TypeCompat VectorType::isCompatible(Operator op, CompiledType* rType) const
{
	if (rType == nullptr)
	{
		return generics[0]->isCompatible(op, nullptr);
	}

	if (rType->category == TypeCategory::VECTOR || rType->category == TypeCategory::PRIMITIVE)
	{
		switch (op)
		{
			case Operator::BIT_AND:
			case Operator::BIT_NOT:
			case Operator::BIT_OR:
			case Operator::BIT_XOR: return TypeCompat::INCOMPATIBLE_OP;
		}

		return TypeCompat::COMPATIBLE;
	}

	return TypeCompat::INCOMPATIBLE_TYPE;
}

uint32_t VectorType::typeDeclSpirV(SpirVAssembler* codeAsm)
{
	if (ssa != 0)
	{
		return ssa;
	}

	uint32_t subSSA = generics[0]->typeDeclSpirV(codeAsm);

	ssa = codeAsm->newAssign();

	codeAsm->pushAll({spirv::OpTypeVector(), ssa, subSSA, vecElements});

	return ssa;
}

uint32_t VectorType::mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const
{
	//TODO implement
	return 0;
}

uint32_t VectorType::mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const
{
	//TODO implement
	return 0;
}

