
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVoid : public ConcreteType
	{
		TypeVoid() : ConcreteType(TypeCategory::VOID, "void") {}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;
		
		void getConvertibleTypes(std::set<ConcreteType*>* types, CaliburnAssembler* codeAsm) override {}

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;

		uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) override;

		uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, ConcreteType* rType, uint32_t rvalueSSA, ConcreteType*& endType) const override;

		uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, ConcreteType*& endType) const override;

	};
}
