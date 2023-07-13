
#pragma once

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <variant>
#include <vector>

#include "ast/generics.h"
#include "ast/symbols.h"

#include "cllr/cllrasm.h"

#include "langcore.h"
#include "syntax.h"

namespace caliburn
{
	struct ParsedType;
	struct RealType;
	struct Variable;
	struct Function;

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

		virtual cllr::TypedSSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const = 0;

	};

	struct Variable : ParsedObject
	{
	protected:
		cllr::SSA id = 0;
		cllr::SSA varType = 0;

	public:
		StmtModifiers mods = {};
		sptr<Token> start = nullptr;
		sptr<Token> nameTkn = nullptr;
		sptr<ParsedType> typeHint = nullptr;
		
		sptr<Value> initValue = nullptr;
		bool isConst = false;

		Variable() = default;
		virtual ~Variable() {}

		sptr<Token> firstTkn() const override
		{
			return start;
		}

		sptr<Token> lastTkn() const override
		{
			return nameTkn;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table);

		virtual cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;

		virtual cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) = 0;

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
		const std::string name;
		const sptr<Token> nameTkn;

		sptr<Token> lastToken = nullptr;

		sptr<GenericArguments> genericArgs;
		std::vector<sptr<Value>> arrayDims;

		ParsedType(std::string n) : name(n), nameTkn(nullptr) {}
		ParsedType(sptr<Token> n) : name(n->str), nameTkn(n) {}
		virtual ~ParsedType() {}

		sptr<Token> firstTkn() const override
		{
			return nameTkn;
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

			return nameTkn;
		}

		void prettyPrint(ref<std::stringstream> ss) const override;

		sptr<RealType> resolve(sptr<const SymbolTable> table);

	};

	using Member = std::variant<std::monostate, sptr<Variable>, sptr<Function>, sptr<Value>>;

	struct BaseType
	{
	public:
		const TypeCategory category;
		const std::string canonName;
	protected:
		const sptr<RealType> defImpl;
		//sptr<BaseType> superType = nullptr;

	public:
		BaseType(TypeCategory c, std::string n, sptr<RealType> defaultImpl) :
			category(c), canonName(n), defImpl(defaultImpl) {}
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

		virtual Member getMember(ref<const std::string> name) const = 0;

		virtual sptr<Function> getConstructor(ref<std::vector<RealType>> args) const = 0;

		virtual sptr<Function> getDestructor() const = 0;

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
	struct GenericType : BaseType, Generic<T>
	{
	public:
		GenericType(TypeCategory c, std::string n, sptr<GenericSignature> sig) :
			BaseType(c, n, nullptr),
			Generic(sig) {}
		virtual ~GenericType() {}

		sptr<RealType> getImpl(sptr<GenericArguments> gArgs) override
		{
			//hoping the cast no-ops out
			//else this is just to help the compiler complain less
			return std::static_pointer_cast<RealType>(this->makeVariant(gArgs));
		}

	};

	struct RealType : cllr::Emitter
	{
	protected:
		//FIXME Saving type IDs here isn't thread-safe. So either use atomics or cache types in the cllr::Assembler.
		//Given this will be read far more than written, using atomics is probably better
		cllr::SSA id = 0;
		std::string fullName = "";
	public:
		std::vector<sptr<Variable>> members;

		const ptr<BaseType> base;
		const sptr<GenericArguments> genArgs;
		
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

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override = 0;

	};

	struct PrimitiveType : BaseType
	{
		const uint32_t width;

		PrimitiveType(TypeCategory c, std::string n, uint32_t bits, sptr<RealType> defaultImpl) : BaseType(c, n, defaultImpl), width(bits) {}
		virtual ~PrimitiveType() {}

		Member getMember(ref<const std::string> name) const override
		{
			return Member();
		}

		sptr<Function> getConstructor(ref<std::vector<RealType>> args) const override
		{
			return nullptr;
		}

		sptr<Function> getDestructor() const override
		{
			return nullptr;
		}

	};

}
