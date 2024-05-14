
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
	/*
	All statements have a corresponding type. This is currently unused.
	*/
	enum class StmtType
	{
		UNKNOWN,

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
		METHOD,
		CONSTRUCTOR,
		DESTRUCTOR,
		
		ENUM,
		ENUM_WRAPPED,

		SCOPE,
		SETTER,
		FUNC_CALL,

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

	/*
	This was previously used for AST validation, but that code is currently non-existant.
	*/
	constexpr auto TOP_STMT_TYPES = {
		StmtType::VARIABLE,
		//StmtType::IF, //conditional compilation
		StmtType::MODULE, StmtType::IMPORT,
		StmtType::FUNCTION,
		StmtType::SHADER,
		StmtType::STRUCT, StmtType::RECORD, StmtType::CLASS,
		//StmtType::ENUM,
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
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Token> last = nullptr;
		std::vector<sptr<Token>> contents;

		Annotation() = default;
		~Annotation() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			ss << '@' << name->str << '(';

			for (auto const& tkn : contents)
			{
				ss << tkn->str;

				if (tkn != contents.back())
				{
					ss << ' ';
				}

			}

			ss << ')';

		}

	};

	/*
	A statement is a conceptual block of code in Caliburn.

	Caliburn separates statements from expressions in part to prevent goofy code.
	*/
	struct Statement : ParsedObject
	{
		const StmtType type;

		StmtModifiers mods = {};
		std::map<std::string, uptr<Annotation>> annotations;

		Statement(StmtType stmtType) : type(stmtType) {}
		virtual ~Statement() {}
		
		ptr<Annotation> getAnnotation(in<std::string> name) const
		{
			auto found = annotations.find(name);

			if (found == annotations.end())
			{
				return nullptr;
			}

			return found->second.get();
		}
		
		virtual bool validate(sptr<ErrorHandler> errors) const
		{
			return false;
		}

		sptr<Token> firstTkn() const override = 0;

		sptr<Token> lastTkn() const override = 0;

		void prettyPrint(out<std::stringstream> ss) const override {}

		//Only used by top-level statements which declare symbols. The rest, like variables, should use declareSymbols() instead
		virtual void declareHeader(sptr<SymbolTable> table, out<ErrorHandler> err) {}

		virtual void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) = 0;

		virtual void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) = 0;

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

}
