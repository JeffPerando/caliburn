
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVector : public ConcreteType
	{
		uint32_t const elements;
		ConcreteType* const inner;

		TypeVector(uint32_t vecElements, ConcreteType* innerType) :
			ConcreteType(TypeCategory::VECTOR, "vec" + vecElements, 1),
			elements(vecElements), inner(innerType)
		{
			setGeneric(0, innerType);

		}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;

		ConcreteType* clone() const override;

		//virtual void getConvertibleTypes(std::set<ConcreteType*>* types) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;

		virtual void getSSAs(cllr::Assembler& codeAsm) override;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

	};

}