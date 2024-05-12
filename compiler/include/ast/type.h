
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
	struct BaseType;
	struct FunctionGroup;
	struct Method;
	struct Variable;

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

		sptr<BaseType> resolveBase(sptr<const SymbolTable> table) const;

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

		virtual ~Variable() = default;

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

}
