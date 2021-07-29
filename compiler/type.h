
#pragma once

#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace caliburn
{
	class SpirVAssembler;

	//Used for resolved types
	enum TypeAttrib
	{
		//has the concept of a negative
		TA_SIGNED =			0b00000001,
		//is an IEEE 16-bit (or higher) floating point
		TA_FLOAT =			0b00000010,
		//is a generic, e.g. array<T> or list<T>
		TA_GENERIC =		0b00000100,
		//is considered a finite set of elements, like a buffer or array
		//can be accessed using [], e.g. v[x]
		TA_COMPOSITE =		0b00001000,
		//is, well, atomic
		TA_ATOMIC =			0b00010000,
		TA_ALL =			0b00011111
	};

	enum class TypeCategory
	{
		PRIMITIVE, VECTOR, MATRIX, ARRAY, POINTER, STRING, CUSTOM
	};
	
	enum class Operator
	{
		//==, >, <
		COMP_EQ, COMP_GT, COMP_LT,
		//>=, <=
		COMP_GTE, COMP_LTE,
		//&&, ||
		COMP_AND, COMP_OR,
		//+, -, *, /
		ADD, SUB, MUL, DIV,
		//%, ^, &, |
		MOD, POW, BIT_AND, BIT_OR,
		//$, ++, //
		BIT_XOR, APPEND, INTDIV,
		//!, ~, -
		BOOL_NOT, BIT_NOT, NEGATE,
		//|x|, x[n]
		ABS, ARRAY_ACCESS
	};

	struct ParsedType
	{
	private:
		std::string fullName;
	public:
		std::string mod;
		std::string name;
		std::vector<ParsedType*> generics;
		
		ParsedType(std::string n) : ParsedType("", n) {}
		ParsedType(std::string m, std::string n) : mod(m), name(n) {}

		virtual ~ParsedType()
		{
			for (ParsedType* type : generics)
			{
				delete type;
			}

		}
		
		bool operator<(const ParsedType& rhs) const
		{
			if (mod < rhs.mod)
			{
				return true;
			}

			if (name < rhs.name)
			{
				return true;
			}

			if (generics.size() < rhs.generics.size())
			{
				return true;
			}

			for (uint32_t i = 0; i < generics.size(); ++i)
			{
				if (generics[i] < rhs.generics[i])
				{
					return true;
				}
			}

			return false;
		}
		
		std::string getFullName()
		{
			if (fullName.length() > 0)
			{
				return fullName;
			}

			std::stringstream ss;

			if (mod.length() > 0)
			{
				ss << mod << ":";
			}

			ss << name;

			if (generics.size() > 0)
			{
				ss << '<';

				for (auto g : generics)
				{
					ss << ' ' << g->getFullName();
				}

				ss << " >";

			}

			ss.str(fullName);

			return fullName;
		}

		ParsedType* addGeneric(ParsedType* s)
		{
			generics.push_back(s);
			return this;
		}
		
	};

	struct CompiledType
	{
		const TypeCategory category;
		const std::string canonName;
		//in bits
		const uint32_t size = 1;
		const uint32_t attribs = 0;

		CompiledType(TypeCategory c, std::string n, uint32_t s, uint32_t a) :
			category(c), canonName(n), size(s), attribs(a) {}

		bool hasA(TypeAttrib a) const
		{
			return (attribs & a) != 0;
		}

		virtual bool isCompatible(Operator op, CompiledType* rType) const = 0;

		virtual uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) const = 0;

		virtual uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA) = 0;

	};

}
