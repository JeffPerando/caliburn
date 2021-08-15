
#pragma once

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct FloatType : public CompiledType
	{
	protected:
		uint32_t const floatBits;
	public:
		FloatType() : FloatType(32) {}
		FloatType(uint32_t s) :
			CompiledType(TypeCategory::PRIMITIVE,
				"float" + s,
				{ TypeAttrib::SIGNED, TypeAttrib::FLOAT }),
			floatBits(s)
		{}

		virtual uint32_t getSizeBytes() const;

		virtual uint32_t getAlignBytes() const;

		virtual CompiledType* clone();

		virtual TypeCompat isCompatible(Operator op, CompiledType* rType) const;

		virtual uint32_t typeDeclSpirV(SpirVAssembler* codeAsm);

		virtual uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const;

		virtual uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const;

	};

}