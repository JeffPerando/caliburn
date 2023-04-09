
#include "typevector.h"

using namespace caliburn;

uint32_t TypeVector::getSizeBytes() const
{
	return inner->getSizeBytes() * elements;
}

uint32_t TypeVector::getAlignBytes() const
{
	return inner->getAlignBytes();
}

ConcreteType* TypeVector::clone() const
{
	return new TypeVector(elements, inner);
}
/*
void TypeVector::getConvertibleTypes(std::set<ConcreteType*>& types)
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
*/
TypeCompat TypeVector::isCompatible(Operator op, ConcreteType* rType) const
{
	if (rType == nullptr)
	{
		return generics[0]->isCompatible(op, nullptr);
	}

	if (rType->category == TypeCategory::VECTOR || rType->category == inner->category)
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

void TypeVector::emitDeclCLLR(cllr::Assembler& codeAsm)
{
	inner->emitDeclCLLR(codeAsm);
	id = codeAsm.push(0, cllr::Opcode::TYPE_VECTOR, { elements }, { inner->id }, true);

}
