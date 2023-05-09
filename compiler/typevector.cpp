
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

sptr<Type> TypeVector::makeVariant(ref<std::vector<sptr<Type>>> genArgs) const
{
	return std::make_shared<TypeVector>(elements, genArgs[0]);
}

TypeCompat TypeVector::isCompatible(Operator op, sptr<Type> rType) const
{
	if (rType == nullptr)
	{
		return inner->isCompatible(op, nullptr);
	}

	if (rType->category == TypeCategory::VECTOR || rType->category == inner->category)
	{
		switch (op)
		{
			case Operator::BIT_AND:
			case Operator::BIT_NEG:
			case Operator::BIT_OR:
			case Operator::BIT_XOR: return TypeCompat::INCOMPATIBLE_OP;
		}

		return TypeCompat::COMPATIBLE;
	}

	return TypeCompat::INCOMPATIBLE_TYPE;
}

void TypeVector::emitDeclCLLR(ref<cllr::Assembler> codeAsm)
{
	inner->emitDeclCLLR(codeAsm);
	id = codeAsm.pushNew(cllr::Opcode::TYPE_VECTOR, { elements }, { inner->id });

}
