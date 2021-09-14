
#pragma once

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct TypeVector : public SpecializedType
	{
	protected:
		uint32_t const vecElements;
	public:
		TypeVector(std::string fullName, uint32_t elements, CompiledType* innerType) :
			SpecializedType(TypeCategory::VECTOR,
				fullName,
				{ TypeAttrib::COMPOSITE, TypeAttrib::GENERIC }, 1),
			vecElements(elements)
		{
			setGeneric(0, innerType);

		}
		
		virtual uint32_t getMandatoryGenericCount()
		{
			return 0;
		}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;

		CompiledType* clone() const override;

		TypeCompat isCompatible(Operator op, CompiledType* rType) const override;

		uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) override;

		uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const override;

		uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const override;

	};

}