
#pragma once

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct TypeVector : public SpecializedType
	{
		uint32_t const elements;

		TypeVector(std::string fullName, uint32_t vecElements, CompiledType* innerType) :
			SpecializedType(TypeCategory::VECTOR,
				fullName,
				{TypeAttrib::COMPOSITE, TypeAttrib::GENERIC}, 1),
			elements(vecElements)
		{
			setGeneric(0, innerType);

		}
		
		virtual uint32_t getMandatoryGenericCount() override
		{
			return 0;
		}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;

		CompiledType* clone() const override;

		void getConvertibleTypes(std::set<CompiledType*>* types, CaliburnAssembler* codeAsm) override;

		TypeCompat isCompatible(Operator op, CompiledType* rType) const override;

		uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) override;

		uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const override;

		uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const override;

	};

}