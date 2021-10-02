
#pragma once

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct TypeFloat : public PrimitiveType
	{
	protected:
		const uint32_t floatBits;
	public:
		TypeFloat() : TypeFloat(32) {}
		TypeFloat(uint32_t s) :
			PrimitiveType(TypeCategory::PRIMITIVE,
				"float" + s,
				(TypeAttrib)(TypeAttrib::SIGNED | TypeAttrib::FLOAT)),
			floatBits(s)
		{}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;
		
		void getConvertibleTypes(std::set<CompiledType*>* types, CaliburnAssembler* codeAsm) override;

		TypeCompat isCompatible(Operator op, CompiledType* rType) const override;

		uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) override;

		uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const override;

		uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const override;

	};

}