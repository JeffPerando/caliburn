
#pragma once

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "ast/generics.h"
#include "ast/symbols.h"

#include "cllr/cllrasm.h"

#include "langcore.h"
#include "syntax.h"

namespace caliburn
{
	class BaseType;
	class RealType;
	class Variable;

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

	struct Value: public ParsedObject
	{
		const ValueType vType;
		//sptr<RealType> type = nullptr;

		Value(ValueType vt) : vType(vt) {}
		virtual ~Value() {}

		virtual bool isLValue() const = 0;

		virtual bool isCompileTimeConst() const = 0;

		//virtual void resolveSymbols(sptr<const SymbolTable> table) = 0;

		virtual cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const = 0;

	};

	class Variable : public ParsedObject
	{
	protected:
		cllr::SSA id = 0;

	public:
		const StmtModifiers mods;
		const sptr<Token> start, name;
		const sptr<ParsedType> typeHint;
		const sptr<Value> initValue;
		
		bool isConst = false;

		Variable(StmtModifiers mods, const ref<sptr<Token>> name, const ref<sptr<ParsedType>> typeHint, const ref<sptr<Value>> initValue) :
			Variable(mods, name, name, typeHint, initValue) {};
		Variable(StmtModifiers mods, const ref<sptr<Token>> start, const ref<sptr<Token>> name, const ref<sptr<ParsedType>> typeHint, const ref<sptr<Value>> initValue) :
			mods(mods),
			start(start),
			name(name),
			typeHint(typeHint),
			initValue(initValue) {}
		virtual ~Variable() {}

		sptr<Token> firstTkn() const override
		{
			return start;
		}

		sptr<Token> lastTkn() const override
		{
			return name;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table);

		virtual cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;

		virtual cllr::SSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) = 0;

		virtual void emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) = 0;

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
		sptr<RealType> resultType = nullptr;
	public:
		const sptr<Token> name;
		sptr<Token> lastToken = nullptr;
		sptr<GenericArguments> genericArgs;
		std::vector<sptr<Value>> arrayDims;
		
		ParsedType() : ParsedType(sptr<Token>(nullptr)) {}
		ParsedType(std::string name) : ParsedType(sptr<Token>(nullptr))
		{
			fullName = name;
		}
		ParsedType(sptr<Token> n) : name(n) {}

		virtual ~ParsedType() {}

		sptr<Token> firstTkn() const override
		{
			return name;
		}

		sptr<Token> lastTkn() const override
		{
			if (lastToken != nullptr)
			{
				return lastToken;
			}

			if (!genericArgs->args.empty())
			{
				return genericArgs->lastTkn();
			}

			return name;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			if (name == nullptr)
			{
				ss << "INVALID TYPE PLS FIX";
				return;
			}

			if (fullName.length() == 0)
			{
				std::stringstream s;

				s << name->str;
				genericArgs->prettyPrint(s);

				s.str(fullName);

			}

			ss << fullName;

		}

		sptr<RealType> resolve(sptr<const SymbolTable> table);

	};

	class BaseType
	{
		sptr<RealType> defImpl;
	public:
		const TypeCategory category;
		const std::string canonName;
	protected:
		//sptr<BaseType> superType = nullptr;

	public:
		BaseType(TypeCategory c, std::string n, sptr<RealType> defaultImpl) : category(c), canonName(n), defImpl(defaultImpl) {}
		virtual ~BaseType() {}

		bool operator!=(ref<const BaseType> rhs) const
		{
			return !(*this == rhs);
		}

		bool operator==(ref<const BaseType> rhs) const
		{
			return canonName == rhs.canonName;
		}

		//virtual void getConvertibleTypes(std::set<sptr<ConcreteType>>& types) = 0;

		//virtual TypeCompat isCompatible(Operator op, sptr<BaseType> rType) const = 0;

		virtual uint64_t parseLiteral(ref<const std::string> lit) const
		{
			throw std::exception("Cannot parse literal for type; Please only attempt parsing literals of ints or floats");
		}

		virtual sptr<RealType> getImpl(sptr<GenericArguments> gArgs)
		{
			if (!gArgs->empty())
			{
				//TODO complain
				return nullptr;
			}

			return defImpl;
		}

		virtual sptr<Variable> getMember(ref<std::string> name) const
		{
			return nullptr;
		}

		/* TODO reconsider inheritance
		sptr<BaseType> getSuper()
		{
			return superType;
		}

		bool isSuperOf(sptr<BaseType> type)
		{
			ptr<BaseType> head = this;

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
		*/
	};

	template<typename T, typename std::enable_if<std::is_base_of<RealType, T>::value>::type* = nullptr>
	class GenericType : public Generic<T>, public BaseType
	{
	public:
		GenericType(TypeCategory c, std::string n, sptr<GenericSignature> sig) :
			BaseType(c, n, nullptr),
			Generic<T>(sig)
		{}

		sptr<RealType> getImpl(sptr<GenericArguments> gArgs) override
		{
			return sptr_demote<T, RealType>(makeVariant(gArgs));
		}

	};

	class RealType : cllr::Emitter
	{
	protected:
		cllr::SSA id = 0;
		std::string fullName = "";
	public:
		std::vector<sptr<Variable>> members;

		const ptr<BaseType> base;
		const sptr<GenericArguments> genArgs;
		const sptr<SymbolTable> memberTable = new_sptr<SymbolTable>();

		RealType(ptr<BaseType> parent, sptr<GenericArguments> gArgs = new_sptr<GenericArguments>()) : base(parent), genArgs(gArgs) {}
		virtual ~RealType() {}

		std::string getFullName()
		{
			if (genArgs->empty())
			{
				return base->canonName;
			}

			if (fullName.length() > 0)
			{
				return fullName;
			}

			std::stringstream ss;

			ss << base->canonName;

			genArgs->prettyPrint(ss);

			fullName = ss.str();

			return fullName;
		}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	template<typename T>
	class PrimitiveType : BaseType
	{
	public:
		const uint32_t width;

		PrimitiveType(TypeCategory c, std::string n, uint32_t bits, sptr<T> defaultImpl) : BaseType(c, n, sptr_demote<T, RealType>(defaultImpl)), width(bits) {}
		virtual ~PrimitiveType() {}

	};

}
