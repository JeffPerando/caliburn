
#pragma once

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct IntType : public CompiledType
	{
		IntType() : IntType(32, true) {}
		IntType(uint32_t b, bool s) :
			CompiledType(TypeCategory::PRIMITIVE,
				(s ? "int" : "uint") + b, b,
				s ? TypeAttrib::TA_SIGNED : 0)
		{}

		virtual TypeCompat isCompatible(Operator op, CompiledType* rType) const;

		virtual uint32_t typeDeclSpirV(SpirVAssembler* codeAsm);

		virtual uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const;

		virtual uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const;

	};

}