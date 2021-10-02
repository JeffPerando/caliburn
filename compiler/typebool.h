
#pragma once

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct TypeBool : public CompiledType
	{
		TypeBool() : CompiledType(TypeCategory::BOOLEAN, "bool", TypeAttrib::NONE){}

		uint32_t getSizeBytes() const override
		{
			return 1;
		}

		uint32_t getAlignBytes() const override
		{
			return 1;
		}

		void getConvertibleTypes(std::set<CompiledType*>* types, CaliburnAssembler* codeAsm) override;

		TypeCompat isCompatible(Operator op, CompiledType* rType) const override;

		uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) override;

		uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const override;

		uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const override;

	};

}
