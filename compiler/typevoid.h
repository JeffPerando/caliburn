
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVoid : public CompiledType
	{
		TypeVoid() : CompiledType(TypeCategory::VOID, "void", {}) {}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;

		TypeCompat isCompatible(Operator op, CompiledType* rType) const override;

		uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) override;

		uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const override;

		uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const override;

	};
}
