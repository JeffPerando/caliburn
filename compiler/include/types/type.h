
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
	struct Variable;
	struct FunctionGroup;
	struct BaseType;

	enum class TypeCategory : uint32_t
	{
		VOID,
		FLOAT,
		INT,
		VECTOR,
		MATRIX,
		ARRAY,
		STRUCT,
		BOOLEAN,
		POINTER,
		TEXTURE
		//TUPLE
		//STRING

	};

	enum class ValueType
	{
		UNKNOWN,

		INT_LITERAL,
		FLOAT_LITERAL,
		STR_LITERAL,
		BOOL_LITERAL,
		ARRAY_LITERAL,
		EXPRESSION,
		CAST,
		SUB_ARRAY,
		VAR_READ,
		MEMBER_READ,
		UNARY_EXPR,
		FUNCTION_CALL,
		SETTER,
		NULL_LITERAL,
		DEFAULT_INIT,
		SIGN,
		UNSIGN

	};

	using ValueResult = std::variant<
		std::monostate,
		cllr::TypedSSA,
		sptr<BaseType>,
		sptr<cllr::LowType>,
		sptr<Module>,
		sptr<FunctionGroup>
	>;

	struct Value : ParsedObject
	{
		const ValueType vType;
		
		Value(ValueType vt) : vType(vt) {}
		virtual ~Value() {}

		virtual bool isLValue() const = 0;

		virtual bool isCompileTimeConst() const = 0;

		virtual ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const = 0;

	};

	struct ParsedType : ParsedObject
	{
	private:
		std::string fullName = "";
	protected:
		sptr<cllr::LowType> resultType = nullptr;
	public:
		const std::string name;
		const sptr<Token> nameTkn;
		const sptr<GenericArguments> genericArgs;

		sptr<Token> lastToken = nullptr;

		std::vector<sptr<Value>> arrayDims;//TODO implement properly

		ParsedType(in<std::string> n) : name(n), nameTkn(nullptr), genericArgs(new_sptr<GenericArguments>()) {}
		ParsedType(sptr<Token> n) : name(n->str), nameTkn(n), genericArgs(new_sptr<GenericArguments>()) {}
		ParsedType(in<std::string> n, sptr<GenericArguments> gArgs) : name(n), nameTkn(nullptr), genericArgs(gArgs) {}
		ParsedType(sptr<Token> n, sptr<GenericArguments> gArgs) : name(n->str), nameTkn(n), genericArgs(gArgs) {}

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

		void prettyPrint(out<std::stringstream> ss) const override;

		sptr<BaseType> resolveBase(sptr<const SymbolTable> table);

		sptr<cllr::LowType> resolve(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm);

		static sptr<ParsedType> parse(in<std::string> str);

	};

	struct ParsedVar
	{
		StmtModifiers mods{};
		sptr<Token> first;
		bool isConst = false;
		sptr<ParsedType> typeHint;
		sptr<Token> name;
		sptr<Value> initValue;

	};

	struct Variable : ParsedObject
	{
	protected:
		cllr::TypedSSA id;

	public:
		const std::string name;

		StmtModifiers mods = {};
		sptr<Token> first = nullptr;
		sptr<Token> nameTkn = nullptr;
		sptr<ParsedType> typeHint = nullptr;

		sptr<Value> initValue = nullptr;
		bool isConst = false;

		Variable(in<std::string> n) : name(n) {}

		Variable(in<ParsedVar> v) : name(v.name->str)
		{
			mods = v.mods;
			isConst = v.isConst;
			first = v.first;
			nameTkn = v.name;
			typeHint = v.typeHint;
			initValue = v.initValue;

		}

		virtual ~Variable() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return nameTkn;
		}

		virtual cllr::TypedSSA emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) = 0;

		virtual cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) = 0;

		virtual void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs) = 0;

	};

	struct BaseType
	{
	public:
		const TypeCategory category;
		const std::string canonName;
		
	public:
		BaseType(TypeCategory c, in<std::string> n) :
			category(c), canonName(n) {}
		virtual ~BaseType() {}

		bool operator!=(in<BaseType> rhs) const
		{
			return !(*this == rhs);
		}

		bool operator==(in<BaseType> rhs) const
		{
			return canonName == rhs.canonName;
		}

		virtual sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;
		
	};

}
