
#pragma once

#include <map>
#include <string>
#include <vector>

#include "assembler.h"
#include "type.h"

namespace caliburn
{
	class CompiledType
	{
		//in bytes
		virtual size_t size() const = 0;

		virtual int32_t attribs() const = 0;

		virtual bool isCompatible(Operator op) const = 0;

		virtual uint32_t typeOpSpirV(SpirVAssembler* codeAsm) const = 0;

		virtual uint32_t mathOpSpirV(Operator op, CompiledType* otherType) const = 0;

	};

	class IntSType : public CompiledType
	{
		size_t intLen;

		IntSType() : IntSType(4) {}
		IntSType(size_t s) : intLen(s) {}

		size_t size() const
		{
			return intLen;
		}

		int32_t attribs() const
		{
			return TypeAttrib::PRIMITIVE | TypeAttrib::SIGNED;
		}

		bool isCompatible(Operator op) const
		{
			return op != Operator::APPEND;
		}

	};

}
