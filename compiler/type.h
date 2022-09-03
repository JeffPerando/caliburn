
#pragma once

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "langcore.h"

namespace caliburn
{
	class CaliburnAssembler;
	class SpirVAssembler;

	enum class TypeAttrib : uint32_t
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
		VOID, BOOLEAN, PRIMITIVE, VECTOR, MATRIX, ARRAY, POINTER, STRUCT
	};
	
	enum class TypeCompat
	{
		COMPATIBLE,
		NEEDS_CONVERSION,
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
				ss << GENERIC_START;

				for (size_t i = 0; i < generics.size(); ++i)
				{
					auto g = generics[i];
					ss << g->getFullName();

					if (i + 1 < generics.size())
					{
						ss << ',';
					}

				}

				ss << GENERIC_END;

			}

			ss.str(fullName);

			return fullName;
		}

		ParsedType* addGeneric(ParsedType* s)
		{
			generics.push_back(s);
			return this;
		}
		
		ConcreteType* resolve(CaliburnAssembler* codeAsm, SymbolTable* syms);

	};

	struct ConcreteType
	{
		const TypeCategory category;
		const std::string canonName;
		const size_t maxGenerics;
	protected:
		//Since every type is made for a particular assembler, storing the SSA is fine
		uint32_t ssa = 0;
		uint32_t attribs = (uint32_t)TypeAttrib::NONE;
		ConcreteType* superType = nullptr;
		std::map<std::string, TypedOffset> fields;
		std::vector<ConcreteType*> generics;
		//TODO add dirty flag to trigger a refresh. that or just control when aspects can be edited
		std::string fullName = "";
	public:
		ConcreteType(TypeCategory c, std::string n, std::initializer_list<TypeAttrib> as = {}, size_t genMax = 0) :
			category(c), canonName(n), maxGenerics(genMax)
		{
			for (auto attrib : as)
			{
				attribs |= (int)attrib;
			}

			if (maxGenerics > 0)
			{
				generics.reserve(maxGenerics);
				attribs |= (int)TypeAttrib::GENERIC;

			}
			
		}

		//annoyed that != doesn't have a default implementation that does this
		bool operator!=(const ConcreteType& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator==(const ConcreteType& rhs) const
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

			if (generics.size() != rhs.generics.size())
			{
				return false;
			}

			for (size_t i = 0; i < generics.size(); ++i)
			{
				ConcreteType* lhsGeneric = generics[i];
				ConcreteType* rhsGeneric = rhs.generics[i];

				if (*lhsGeneric == *rhsGeneric)
				{
					continue;
				}

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

		std::string getFullName()
		{
			if (maxGenerics == 0)
			{
				return canonName;
			}

			if (fullName.length() > 0)
			{
				return fullName;
			}

			std::stringstream ss;

			ss << canonName;
			ss << GENERIC_START;
			for (uint32_t i = 0; i < generics.size(); ++i)
			{
				ss << generics[i]->getFullName();
			}
			ss << GENERIC_END;

			fullName = ss.str();

			return fullName;
		}

		ConcreteType* getSuper()
		{
			return superType;
		}

		bool isSuperOf(ConcreteType* type)
		{
			ConcreteType* head = this;

			while (head)
			{
				if (type == head)
				{
					return true;
				}

				head = head->superType;

			}

			return false;
		}

		virtual void setGeneric(size_t index, ConcreteType* type)
		{
			if (index >= maxGenerics)
			{
				throw std::exception("cannot add a generic to this type!");
			}

			generics[index] = type;

		}

		virtual TypedSSA* getField(std::string name)
		{
			return nullptr;
		}

		virtual bool declareField(std::string name, TypedSSA* data)
		{
			return false;
		}

		//NOTE: because the size can depend on things like generics, members, etc., this HAS to be a method
		virtual uint32_t getSizeBytes() const = 0;

		//Conveniently, for most all primitives, alignment == size
		virtual uint32_t getAlignBytes() const = 0;

		//ONLY exists for making a new generic form. so if you don't use generics, there's ZERO point in
		//properly implementing this.
		virtual ConcreteType* clone() const
		{
			return (ConcreteType*)this;
		}

		virtual void getConvertibleTypes(std::set<ConcreteType*>* types, CaliburnAssembler* codeAsm) = 0;

		virtual TypeCompat isCompatible(Operator op, ConcreteType* rType) const = 0;

		virtual uint32_t typeDeclSpirV(SpirVAssembler* codeAsm) = 0;

		virtual uint32_t mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, ConcreteType* rType, uint32_t rvalueSSA, ConcreteType*& endType) const = 0;

		//used for BIT_NOT, NEGATE, ABS
		virtual uint32_t mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, ConcreteType*& endType) const = 0;

	};

}
