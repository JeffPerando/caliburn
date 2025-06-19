
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
	struct ParsedType : ParsedObject
	{
	private:
		std::string fullName = "";
	public:
		const std::string_view name;
		const Token nameTkn;
		const sptr<GenericArguments> genericArgs;

		Token lastToken;

		std::vector<sptr<Expr>> arrayDims;//TODO implement properly

		ParsedType(std::string_view n) : name(n), genericArgs(new_sptr<GenericArguments>()) {}
		ParsedType(in<Token> n) : name(n.str), nameTkn(n), genericArgs(new_sptr<GenericArguments>()) {}
		ParsedType(std::string_view n, sptr<GenericArguments> gArgs) : name(n), genericArgs(gArgs) {}
		ParsedType(in<Token> n, sptr<GenericArguments> gArgs) : name(n.str), nameTkn(n), genericArgs(gArgs) {}

		virtual ~ParsedType() = default;

		Token firstTkn() const noexcept override
		{
			return nameTkn;
		}

		Token lastTkn() const noexcept override
		{
			if (lastToken.type != TokenType::UNKNOWN)
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

		sptr<cllr::LowType> resolve(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const;

		static sptr<ParsedType> parse(in<std::string> str);

	};

	struct ParsedVar
	{
		ExprModifiers mods{};
		Token first;
		bool isConst = false;
		sptr<ParsedType> typeHint;
		Token name;
		sptr<Expr> initValue;

	};

	struct Variable : ParsedObject
	{
	public:
		const std::string_view name;

		ExprModifiers mods = {};
		Token first;
		Token nameTkn;
		sptr<ParsedType> typeHint = nullptr;

		sptr<Expr> initValue = nullptr;
		bool isConst = false;

		Variable(std::string_view n) : name(n) {}

		Variable(in<ParsedVar> v) : name(v.name.str)
		{
			mods = v.mods;
			isConst = v.isConst;
			first = v.first;
			nameTkn = v.name;
			typeHint = v.typeHint;
			initValue = v.initValue;

		}

		virtual ~Variable() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return nameTkn;
		}
		
		virtual cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) = 0;

		virtual void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs) = 0;

	protected:
		virtual cllr::TypedSSA emitVarCLLR(sptr<const SymbolTable> table, bool isBeingWritten, out<cllr::Assembler> codeAsm) = 0;


	};

}
