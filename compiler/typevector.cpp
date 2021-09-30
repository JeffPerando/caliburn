
#include "allasm.h"
#include "typevector.h"

using namespace caliburn;

uint32_t TypeVector::getSizeBytes() const
{
	return elements * generics[0]->getSizeBytes();
}

uint32_t TypeVector::getAlignBytes() const
{
	return generics[0]->getAlignBytes();
}

CompiledType* TypeVector::clone() const
{
	return new TypeVector(elements, generics[0]);
}

void TypeVector::getConvertibleTypes(std::set<CompiledType*>* types, CaliburnAssembler* codeAsm)
{
	auto vecs = codeAsm->getAllVecTypes();

	for (auto vt : *vecs)
	{
		if (vt->elements < elements)
		{
			types->emplace(vt);

		}

	}

}

TypeCompat TypeVector::isCompatible(Operator op, CompiledType* rType) const
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

uint32_t TypeVector::typeDeclSpirV(SpirVAssembler* codeAsm)
{
	if (ssa != 0)
	{
		return ssa;
	}

	uint32_t subSSA = generics[0]->typeDeclSpirV(codeAsm);

	ssa = codeAsm->newAssign();

	codeAsm->pushAll({spirv::OpTypeVector(), ssa, subSSA, elements});

	return ssa;
}

uint32_t TypeVector::mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const
{
	//TODO implement
	return 0;
}

uint32_t TypeVector::mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const
{
	//TODO implement
	return 0;
}

