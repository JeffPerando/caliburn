
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVoid : public CompiledType
	{
		TypeVoid() : CompiledType(TypeCategory::VOID, "void", {}) {}

		virtual uint32_t getSizeBytes() const;

		//Conveniently, for most all primitives, alignment == size
		virtual uint32_t getAlignBytes() const;

		virtual CompiledType* clone();

		virtual TypeCompat isCompatible(Operator op, CompiledType* rType) const;

		virtual uint32_t typeDeclSpirV(SpirVAssembler* codeAsm);

		virtual uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const;

		//used for BIT_NOT, NEGATE, ABS
		virtual uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const;

	};
}
