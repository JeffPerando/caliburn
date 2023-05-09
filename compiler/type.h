
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
#include "syntax.h"

namespace caliburn
{
	struct ParsedType;
	struct Type;

	enum class ValueType
	{
		UNKNOWN,

		INT_LITERAL,
		FLOAT_LITERAL,
		STR_LITERAL,
		EXPRESSION,
		VAR_READ,
		SUB_ARRAY,
		CAST,
		FUNCTION_CALL,
		DEFAULT_INIT
	};

	struct Value : public ParsedObject
	{
		const ValueType vType;
		sptr<Type> type = nullptr;

		Value(ValueType vt) : vType(vt) {}
		virtual ~Value() {}

		virtual bool isLValue() const = 0;

		virtual bool isCompileTimeConst() const = 0;

		virtual void resolveSymbols(sptr<const SymbolTable> table) = 0;

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const = 0;

	};

	struct Variable : public ParsedObject
	{
		cllr::SSA id = 0;
		StorageModifiers storeMods = {};
		bool isConst = false;
		sptr<Token> name = nullptr;
		sptr<Type> type = nullptr;

		uptr<ParsedType> typeHint = nullptr;
		uptr<Value> initValue = nullptr;
		
		Variable() {}
		Variable(sptr<Token> varName, uptr<ParsedType> hint, uptr<Value> init, bool isImmut) :
			name(varName),
			typeHint(std::move(hint)),
			initValue(std::move(init)),
			isConst(isImmut) {}
		virtual ~Variable() {}

		virtual void resolveSymbols(sptr<const SymbolTable> table);

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) = 0;

		virtual cllr::SSA emitLoadCLLR(ref<cllr::Assembler> codeAsm, cllr::SSA target) = 0;

		virtual void emitStoreCLLR(ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) = 0;

	};

	enum class TypeCategory
	{
		VOID, FLOAT, INT, VECTOR, MATRIX, ARRAY, STRUCT, BOOLEAN, POINTER, TUPLE, STRING
	};
	
	enum class TypeCompat
	{
		COMPATIBLE,
		NEEDS_CONVERSION,
		INCOMPATIBLE_TYPE,
		INCOMPATIBLE_OP
	};

	struct ParsedType : public ParsedObject
	{
	private:
		std::string fullName = "";
	protected:
		sptr<Type> resultType = nullptr;
	public:
		const sptr<Token> mod;
		const sptr<Token> name;
		sptr<Token> lastToken = nullptr;
		std::vector<uptr<ParsedType>> generics;
		std::vector<uptr<Value>> arrayDims;
		
		ParsedType() : ParsedType(sptr<Token>(nullptr)) {}
		ParsedType(std::string name) : ParsedType(sptr<Token>(nullptr))
		{
			fullName = name;
		}
		ParsedType(sptr<Token> n) : ParsedType(nullptr, n) {}
		ParsedType(sptr<Token> m, sptr<Token> n) : mod(m), name(n) {}

		virtual ~ParsedType() {}

		sptr<Token> firstTkn() const override
		{
			if (mod != nullptr)
				return mod;
			return name;
		}

		sptr<Token> lastTkn() const override
		{
			if (lastToken != nullptr)
			{
				return lastToken;
			}

			if (!generics.empty())
			{
				return generics.back()->lastTkn();
			}

			return name;
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
					auto const& g = generics[i];
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

		void addGeneric(uptr<ParsedType> s)
		{
			generics.push_back(std::move(s));
		}

		/*
		TODO check for memory leaks; this part involves cloning.
		*/
		sptr<Type> resolve(sptr<const SymbolTable> table);

	};

	struct Type : public cllr::Emitter
	{
		cllr::SSA id = 0;
		const TypeCategory category;
		const std::string canonName;
		const size_t maxGenerics;
		const size_t minGenerics;
		const sptr<SymbolTable> memberTable = std::make_shared<SymbolTable>();
	protected:
		sptr<Type> superType = nullptr;
		std::vector<sptr<Type>> generics, genericDefaults;
		std::vector<uptr<Variable>> members;

		//TODO add dirty flag to trigger a refresh. that or just control when aspects can be edited
		std::string fullName = "";
	public:
		Type(TypeCategory c, std::string n, size_t genMax = 0, size_t genMin = 0) :
			category(c), canonName(n), maxGenerics(genMax), minGenerics(genMin)
		{
			if (minGenerics > maxGenerics)
			{
				throw std::make_unique<std::exception>("Invalid concrete type; More generics required than alotted");
			}

			if (maxGenerics > 0)
			{
				generics.reserve(maxGenerics);

			}
			
		}

		virtual ~Type() {}

		//annoyed that != doesn't have a default implementation that does this
		bool operator!=(ref<const Type> rhs) const
		{
			return !(*this == rhs);
		}

		bool operator==(ref<const Type> rhs) const
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
				auto const& lhsGeneric = generics[i];
				auto const& rhsGeneric = rhs.generics[i];

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

		sptr<Type> getSuper()
		{
			return superType;
		}

		bool isSuperOf(sptr<Type> type)
		{
			ptr<Type> head = this;

			while (head)
			{
				if (type.get() == head)
				{
					return true;
				}

				head = head->superType.get();

			}

			return false;
		}

		virtual void setGeneric(size_t index, sptr<Type> type)
		{
			if (index >= maxGenerics)
			{
				//TODO compiler error
				throw std::exception("cannot add a generic to this type!");
			}

			generics[index] = type;

		}

		virtual cllr::SSA emitDefaultInitValue(ref<cllr::Assembler> codeAsm) = 0;

		//NOTE: because the size can depend on things like generics, members, etc., this HAS to be a method
		virtual uint32_t getSizeBytes() const = 0;

		//Conveniently, for most all primitives, alignment == size
		virtual uint32_t getAlignBytes() const
		{
			return getSizeBytes();
		}
		
		//virtual void getConvertibleTypes(std::set<Concreteptr<Type>>& types) = 0;

		virtual TypeCompat isCompatible(Operator op, sptr<Type> rType) const = 0;

		virtual sptr<Type> makeVariant(ref<std::vector<sptr<Type>>> genArgs) const
		{
			throw std::exception("Cannot make variant of unspecified type");
		}

		virtual uint64_t parseLiteral(std::string lit) const
		{
			throw std::exception("Cannot parse literal for type; Please only attempt parsing literals of ints or floats");
		}

	};

}
