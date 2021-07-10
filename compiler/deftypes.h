
#pragma once

#include <map>
#include <string>
#include <vector>

#include "spirv.h"

namespace caliburn
{
	enum TypeAttrib
	{
		//simple data types like integers
		PRIMITIVE =		0b00000001,
		//has the concept of a negative
		SIGNED =		0b00000010,
		//is an IEEE 16-bit (or higher) floating point
		FLOAT =			0b00000100,
		//is a generic, e.g. array<T> or list<T>
		GENERIC =		0b00001000,
		//is considered a finite set of elements, like a buffer or array
		//can be accessed using [], e.g. v[x]
		COMPOSITE =		0b00010000,
		//is, well, atomic
		ATOMIC =		0b00100000
	};

	struct TypeData
	{
		std::string const name;
		uint32_t const attribs;
		//in BYTES. B Y T E S (and no I don't expect a 4 MB+ data type)
		uint32_t const typicalSize;
		uint32_t const defOp;
		
		TypeData(std::string n, uint32_t a, uint32_t s, uint32_t op) :
			name(n), attribs(a), typicalSize(s), defOp(op) {}

	};

	struct GenericType : public TypeData
	{
		//pls treat as read-only ty
		std::vector<TypeData*> generics = std::vector<TypeData*>(4);

		GenericType(std::string n, uint32_t a, uint32_t s, uint32_t op, std::vector<TypeData*>* gs) :
			TypeData(n, a | GENERIC, s, op)
		{
			generics.insert(generics.end(), gs->begin(), gs->end());

		}
	};

	class IntType : public TypeData
	{
		IntType(std::string n, uint32_t a, uint32_t s) :
			TypeData(n, a, s, spirv::OpTypeInt()) {}
	};

	static const std::map<std::string, TypeData> defaultTypes = {
		{"int", TypeData("int", PRIMITIVE | SIGNED, 4, spirv::OpTypeInt())}
	};

}
