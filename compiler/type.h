
#pragma once

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "langcore.h"
#include "tokenizer.h"

namespace caliburn
{
	class CaliburnAssembler;
	class SpirVAssembler;

	//Used for resolved types
	enum TypeAttrib
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

	struct CompiledType : public SymbolTable
	{
		const TypeCategory category;
		const std::string canonName;
		const size_t maxGenerics;
	protected:
		//Since every type is made for a particular assembler, storing the SSA is fine
		uint32_t ssa = 0;
		int attribs = TypeAttrib::NONE;
		CompiledType* superType = nullptr;
		std::vector<CompiledType*> generics;
		//TODO add dirty flag to trigger a refresh. that or just control when aspects can be edited
		std::string fullName = "";
	public:
		CompiledType(TypeCategory c, std::string n, int as, size_t genMax = 0) :
			category(c), canonName(n), attribs(as & TypeAttrib::ALL), maxGenerics(genMax)
		{
			if (maxGenerics > 0)
			{
				generics.reserve(maxGenerics);
				attribs |= TypeAttrib::GENERIC;

			}
			
		}

		//annoyed that != doesn't have a default implementation that does this
		bool operator!=(const CompiledType& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator==(const CompiledType& rhs) const
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
				CompiledType* lhsGeneric = generics[i];
				CompiledType* rhsGeneric = rhs.generics[i];

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
			return (attribs & a) != 0;
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
			ss << '[';
			for (uint32_t i = 0; i < generics.size(); ++i)
			{
				ss << generics[i]->getFullName();
			}
			ss << ']';

			fullName = ss.str();

			return fullName;
		}

		CompiledType* getSuper()
		{
			return superType;
		}

		bool isSuperOf(CompiledType* type)
		{
			CompiledType* head = type;

			while (head)
			{
				if (head == this)
				{
					return true;
				}

				head = head->superType;

			}

			return false;
		}

		virtual void setGeneric(size_t index, CompiledType* type)
		{
			if (index >= maxGenerics)
			{
				throw std::exception("cannot add a generic to this type!");
			}

			generics[index] = type;

		}

		//NOTE: because the size can depend on things like generics, members, etc., this HAS to be a method
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

	struct PrimitiveType : public CompiledType
	{
		PrimitiveType(TypeCategory c, std::string n, TypeAttrib as) :
			CompiledType(c, n, as, 0) {}

		virtual bool add(Symbol* symbol) override
		{
			return false;
		}

		virtual Symbol* resolve(std::string name) override
		{
			return nullptr;
		}

	};

}
