
#pragma once

#include <map>
#include <set>
#include <stdint.h>
#include <vector>

#include "basic.h"
#include "error.h"
#include "langcore.h"
#include "syntax.h"

#include "ast/module.h"
#include "ast/symbols.h"
#include "ast/type.h"

#include "cllr/cllrasm.h"

namespace caliburn
{
	enum class ExprType
	{
		UNKNOWN,

		//TODO reconsider
		VARIABLE,

		//Flow control
		IF,
		FOR,
		FORALL,
		WHILE,
		DOWHILE,
		SWITCH,
		CASE,

		MODULE,
		IMPORT,

		TYPEDEF,

		SHADER,
		SHADER_STAGE,

		FUNCTION,
		
		STRUCT,
		RECORD,
		CLASS,
		
		ENUM,
		ENUM_WRAPPED,

		SCOPE,

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

	/*
	This was previously used for AST validation, but that code is currently non-existant.
	*/
	constexpr auto TOP_STMT_TYPES = {
		ExprType::VARIABLE,
		//StmtType::IF, //conditional compilation
		ExprType::MODULE, ExprType::IMPORT,
		ExprType::FUNCTION,
		ExprType::SHADER,
		ExprType::STRUCT, ExprType::RECORD, ExprType::CLASS,
		//TODO enums aren't implemented yet
		//ExprType::ENUM
	};

	/*
	Annotations are a way to add metadata to a statement in Caliburn. They're similar to annotation in Java, but the
	tokens within can be interpreted by the compiler in any way it likes. An invalid annotation can be safely ignored.
	
	Annotations start with an @ symbol and an identifier. If an open parentheses follows, then all symbols before the
	close parentheses will be included. If more open parentheses are found, then the parser will look for matching close
	parentheses for them. In this way, expressions can be added to an annotation. So this:

	@Some(()

	is invalid, since the parentheses are uneven. And this:

	@Other(())

	is valid.

	Annotations can have any identifier, minus reserved keywords, and are only visible to the compiler, not to user code.
	*/
	struct Annotation : ParsedObject
	{
		const Token first;
		const Token name;
		const std::vector<Token> contents;
		const Token last;

		Annotation(in<Token> f, in<Token> n, in<std::vector<Token>> con, in<Token> l) :
			first(f), name(n), contents(con), last(l) {}
		
		Annotation(in<Token> f, in<Token> n, in<Token> l) :
			first(f), name(n), last(l) {}

		virtual ~Annotation() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return last;
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			ss << '@' << name.str << '(';

			for (auto const& tkn : contents)
			{
				ss << tkn.str;

				if (&tkn != &contents.back())
				{
					ss << ' ';
				}

			}

			ss << ')';

		}

	};

	using ValueResult = std::variant<
		std::monostate,
		cllr::TypedSSA,
		sptr<BaseType>,
		sptr<cllr::LowType>,
		sptr<Module>,
		sptr<FunctionGroup>
	>;

	struct Expr : ParsedObject
	{
		const ExprType type;

		ExprModifiers mods = {};
		std::map<std::string, uptr<Annotation>> annotations;

		Expr(ExprType t) : type(t) {}
		virtual ~Expr() = default;

		Token firstTkn() const noexcept override = 0;

		Token lastTkn() const noexcept override = 0;

		void prettyPrint(out<std::stringstream> ss) const override {}

		virtual bool isLValue() const
		{
			return false;
		}

		virtual bool isCompileTimeConst() const
		{
			return false;
		}

		//Only used by top-level statements which declare symbols. The rest, like variables, should use declareSymbols() instead
		virtual void declareHeader(sptr<SymbolTable> table, out<ErrorHandler> err) {}

		virtual ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const = 0;

	};

}
