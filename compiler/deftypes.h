
#pragma once

#include <map>
#include <string>
#include <vector>

#include "assembler.h"
#include "type.h"

namespace caliburn
{
	struct CompiledType
	{
		std::string canonName;
		//in bytes
		size_t size;
		int32_t attribs;

		virtual bool isCompatible(Operator op) const = 0;

		virtual uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) const = 0;

		virtual uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA) const = 0;

	};

	struct IntSType : public CompiledType
	{
		IntSType() : IntSType(4) {}
		IntSType(size_t s)
		{
			size = s;
			canonName = "int" + (s * 8);
			attribs = TypeAttrib::PRIMITIVE | TypeAttrib::SIGNED;
		}

		bool isCompatible(Operator op, CompiledType* rType) const
		{
			return op != Operator::APPEND;
		}

	};

}
