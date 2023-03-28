
#pragma once

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "cllrasm.h"
#include "langcore.h"
#include "symbols.h"

namespace caliburn
{
	struct ParsedType;
	struct ConcreteType;
	struct Statement;

	enum class ValueType
	{
		LITERAL,
		EXPRESSION,
		VARIABLE,
		FUNCTION_CALL
	};

	struct Value : public ParsedObject, public cllr::Emitter
	{
		ValueType const vType;
		ConcreteType* type = nullptr;

		cllr::SSA id = 0;

		Value(ValueType vt) : vType(vt) {}
		virtual ~Value() {}

		virtual bool isLValue() const = 0;

		virtual bool isCompileTimeConst() const = 0;

		virtual void resolveSymbols(const SymbolTable& table) = 0;

		virtual cllr::SSA emitLoadCLLR(cllr::Assembler& codeAsm) = 0;

		virtual void emitStoreCLLR(cllr::Assembler& codeAsm, cllr::SSA value) = 0;

	};

	struct Variable : public Value
	{
		Statement* const owner;

		cllr::SSA id = 0;
		bool isConst = false;
		Token* name = nullptr;
		ParsedType* typeHint = nullptr;
		Value* initValue = nullptr;
		
		Variable() : Value(ValueType::VARIABLE), owner(nullptr) {};
		Variable(Statement* parent, Token* varName, ParsedType* hint, Value* init, bool isImmut) :
			Value(ValueType::VARIABLE),
			owner(parent),
			name(varName),
			typeHint(hint),
			initValue(init),
			isConst(isImmut) {}
		Variable(const Variable& rhs) : Value(ValueType::VARIABLE), owner(rhs.owner)
		{
			id = rhs.id;
			name = rhs.name;
			typeHint = rhs.typeHint;
			type = rhs.type;
			initValue = rhs.initValue;
			isConst = rhs.isConst;

		}
		virtual ~Variable() {}

		virtual bool isCompileTimeConst() const
		{
			return false;
		}

		virtual void getSSAs(cllr::Assembler& codeAsm) override;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

		virtual cllr::SSA emitLoadCLLR(cllr::Assembler& codeAsm) override = 0;

		virtual void emitStoreCLLR(cllr::Assembler& codeAsm, cllr::SSA value) override = 0;

	};

	enum class TypeCategory
	{
		VOID, FLOAT, INT, VECTOR, MATRIX, ARRAY, STRUCT, BOOLEAN, POINTER, TUPLE
	};
	
	enum class TypeCompat
	{
		COMPATIBLE,
		NEEDS_CONVERSION,
		INCOMPATIBLE_TYPE,
		INCOMPATIBLE_OP
	};

	struct TypeConvertResult
	{
		TypeCompat compat;
		ConcreteType* commonType;

	};
	
	struct ParsedType : public ParsedObject
	{
	private:
		std::string fullName = "";
		ConcreteType* resultType = nullptr;
	public:
		Token* const mod;
		Token* const name;
		std::vector<ParsedType*> generics;
		
		ParsedType() : ParsedType(nullptr) {}
		ParsedType(std::string name) : ParsedType(nullptr)
		{
			fullName = name;
		}
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

		Token* firstTkn() const override
		{
			if (mod != nullptr)
				return mod;
			return name;
		}

		Token* lastTkn() const override
		{
			if (!generics.empty())
				return generics.back()->lastTkn();
			return name;
		}
		
		/*
		TODO check for memory leaks; this part involves cloning.
		*/
		ConcreteType* resolve(const SymbolTable& table);

	};

	struct ConcreteType : public cllr::Emitter
	{
		cllr::SSA id = 0;
		const TypeCategory category;
		const std::string canonName;
		const size_t maxGenerics;
		const size_t minGenerics;
	protected:
		ConcreteType* superType = nullptr;
		std::vector<ConcreteType*> generics;
		std::vector<Variable*> vars;

		//TODO add dirty flag to trigger a refresh. that or just control when aspects can be edited
		std::string fullName = "";
	public:
		ConcreteType(TypeCategory c, std::string n, size_t genMax = 0, size_t genMin = 0) :
			category(c), canonName(n), maxGenerics(genMax), minGenerics(genMin)
		{
			if (maxGenerics > 0)
			{
				generics.reserve(maxGenerics);

			}
			
		}
		virtual ~ConcreteType() {}

		//annoyed that != doesn't have a default implementation that does this
		bool operator!=(const ConcreteType& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator==(const ConcreteType& rhs) const
		{
			if (canonName != rhs.canonName)
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
				//TODO compiler error
				throw std::exception("cannot add a generic to this type!");
			}

			generics[index] = type;

		}

		virtual Value* defaultInitValue()
		{
			return nullptr;
		}

		//ONLY exists for making a new generic type. so if you don't use generics, there's ZERO point in
		//properly implementing this. So just return this and be done with it.
		virtual ConcreteType* clone() const = 0;

		//NOTE: because the size can depend on things like generics, members, etc., this HAS to be a method
		virtual uint32_t getSizeBytes() const = 0;

		//Conveniently, for most all primitives, alignment == size
		virtual uint32_t getAlignBytes() const
		{
			return getSizeBytes();
		}
		
		//virtual void getConvertibleTypes(std::set<ConcreteType*>& types) = 0;

		virtual TypeCompat isCompatible(Operator op, ConcreteType* rType) const = 0;

	};

}
