
#pragma once

#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace caliburn
{
	//Used for resolved types
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
		ATOMIC =		0b00100000,
		ALL =			0b00111111
	};

	enum Operator
	{
		//==, >, <
		EQUALS, GREATER, LESSER,
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

	struct TypeData
	{
	private:
		std::string fullName;
	public:
		std::string mod;
		std::string name;
		std::vector<TypeData*> generics;
		
		~TypeData()
		{
			for (TypeData* type : generics)
			{
				delete type;
			}

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

	};

}
