
#pragma once

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "tokenizer.h"

namespace caliburn
{
	class CaliburnAssembler;
	class SpirVAssembler;

	//Used for resolved types
	enum class TypeAttrib
	{
		NONE =			0,
		//has the concept of a negative
		SIGNED =		0b00000001,
		//is an IEEE 16-bit (or higher) floating point
		FLOAT =			0b00000010,
		//is a generic, e.g. array<T> or list<T>
		GENERIC =		0b00000100,
		//is considered a finite set of elements, like a buffer or array
		//can be accessed using [], e.g. v[x]
		COMPOSITE =		0b00001000,
		//is, well, atomic
		ATOMIC =		0b00010000,
		ALL =			0b00011111
	};

	enum class TypeCategory
	{
		VOID, BOOLEAN, PRIMITIVE, VECTOR, MATRIX, ARRAY, POINTER, STRING, CUSTOM
	};
	
	enum class TypeCompat
	{
		COMPATIBLE,
		INCOMPATIBLE_TYPE,
		INCOMPATIBLE_OP
	};

	struct ParsedType
	{
	private:
		std::string fullName;
	public:
		Token* mod;
		Token* name;
		std::vector<ParsedType*> generics;
		
		ParsedType() : ParsedType(nullptr) {}
		ParsedType(Token* n) : ParsedType(nullptr, n) {}
		ParsedType(Token* m, Token* n) : mod(m), name(n) {}

		virtual ~ParsedType()
		{
			for (ParsedType* type : generics)
			{
				delete type;
			}

		}

		std::string getBasicName()
		{
			if (mod)
			{
				return (std::stringstream() << mod->str << ':' << name->str).str();
			}

			return name->str;
		}
		
		std::string getFullName()
		{
			if (fullName.length() > 0)
			{
				return fullName;
			}

			if (name == nullptr)
			{
				return "INVALID TYPE PLS FIX";
			}

			std::stringstream ss;

			if (mod != nullptr)
			{
				ss << mod->str << ':';
			}

			ss << name->str;

			if (generics.size() > 0)
			{
				ss << '[';

				for (size_t i = 0; i < generics.size(); ++i)
				{
					auto g = generics[i];
					ss << g->getFullName();

					if (i + 1 < generics.size())
					{
						ss << ',';
					}

				}

				ss << "]";

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
	protected:
		//Since every type is made for a particular assembler, storing the SSA is fine
		CompiledType* superType = nullptr;
		uint32_t ssa = 0;
		uint32_t attribs = 0;
	public:
		CompiledType(TypeCategory c, std::string n, std::initializer_list<TypeAttrib> as) :
			category(c), canonName(n)
		{
			for (auto a : as)
			{
				attribs |= (int)a;
			}

		}

		//annoyed that != doesn't have a default implementation that does this
		virtual bool operator!=(CompiledType& rhs)
		{
			return !(*this == rhs);
		}

		virtual bool operator==(CompiledType& rhs)
		{
			//cheap hack, sorry
			if (ssa != 0 && rhs.ssa != 0)
			{
				return ssa == rhs.ssa;
			}
			
			if (canonName != rhs.canonName)
			{
				return false;
			}

			if (attribs != rhs.attribs)
			{
				return false;
			}

			return true;
		}

		bool hasA(TypeAttrib a) const
		{
			return (attribs & (int)a) != 0;
		}

		uint32_t getSSA()
		{
			return ssa;
		}

		virtual std::string getFullName()
		{
			return canonName;
		}

		CompiledType* getSuper()
		{
			return superType;
		}

		virtual void setGeneric(size_t index, CompiledType* type)
		{
			throw std::exception("cannot add a generic to this type!");
		}

		virtual uint32_t getSizeBytes() const = 0;

		//Conveniently, for most all primitives, alignment == size
		virtual uint32_t getAlignBytes() const = 0;

		//ONLY exists for making a new generic form. so if you don't use generics, there's ZERO point in
		//properly implementing this.
		virtual CompiledType* clone() const
		{
			return (CompiledType*)this;
		}

		virtual void getConvertibleTypes(std::set<CompiledType*>* types, CaliburnAssembler* codeAsm) = 0;

		virtual TypeCompat isCompatible(Operator op, CompiledType* rType) const = 0;

		virtual uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) = 0;

		virtual uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const = 0;

		//used for BIT_NOT, NEGATE, ABS
		virtual uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const = 0;

	};

	struct SpecializedType : public CompiledType
	{
	protected:
		size_t const genericLimit;
		std::vector<CompiledType*> generics;
	
	public:
		SpecializedType(TypeCategory cat, std::string name,
			std::initializer_list<TypeAttrib> attribs, size_t genericCount = 1) :
			CompiledType(cat, name, attribs), genericLimit(genericCount)
		{
			generics.reserve(genericCount);

		}

		bool operator==(CompiledType& other)
		{
			if (!CompiledType::operator==(other))
			{
				return false;
			}

			auto rhs = (SpecializedType*)&other;

			if (generics.size() != rhs->generics.size())
			{
				return false;
			}

			for (size_t i = 0; i < generics.size(); ++i)
			{
				CompiledType* lhsGeneric = generics[i];
				CompiledType* rhsGeneric = rhs->generics[i];

				if (*lhsGeneric == *rhsGeneric)
				{
					continue;
				}

				return false;
			}

			return true;
		}

		virtual std::string getFullName()
		{
			std::stringstream ss;

			ss << canonName;
			ss << '<';
			for (uint32_t i = 0; i < generics.size(); ++i)
			{
				ss << generics[i]->getFullName();
			}
			ss << '>';

			return ss.str();
		}

		virtual void setGeneric(size_t index, CompiledType* type)
		{
			if (index >= generics.size())
			{
				throw std::exception("Too many generic arguments!");
			}

			generics[index] = type;

		}

		virtual uint32_t getMandatoryGenericCount() = 0;

	};

}
