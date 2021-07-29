
#pragma once

#include <map>
#include <string>
#include <vector>

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	class SpirVAssembler;

	struct IntType : public CompiledType
	{
		IntType() : IntType(32) {}
		IntType(uint32_t s, bool u = false) : 
			CompiledType(TypeCategory::PRIMITIVE,
				(u ? "uint" : "int") + s, s,
				u ? TypeAttrib::TA_SIGNED : 0)
		{}

		virtual bool isCompatible(Operator op, CompiledType* rType) const;

		virtual uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) const;

		virtual uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA);

		virtual uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa) = 0;

	};

}
