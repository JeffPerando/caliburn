
#pragma once

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct TypeVector : public ConcreteType
	{
		uint32_t const elements;

		TypeVector(uint32_t vecElements, ConcreteType* innerType) :
			ConcreteType(TypeCategory::VECTOR,
				"vec" + vecElements, { TypeAttrib::COMPOSITE, TypeAttrib::GENERIC }, 1),
			elements(vecElements)
		{
			setGeneric(0, innerType);

		}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;

		ConcreteType* clone() const override;

		void getConvertibleTypes(std::set<ConcreteType*>* types, CaliburnAssembler* codeAsm) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;

		uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) override;

		uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, ConcreteType* rType, uint32_t rvalueSSA, ConcreteType*& endType) const override;

		uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, ConcreteType*& endType) const override;

	};

}