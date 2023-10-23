
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

#include "cllr/cllrasm.h"

#include "types/type.h"

namespace caliburn
{
	/*
	All statements have a corresponding type. This is currently unused.
	*/
	enum class StatementType
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

	/*
	This was previously used for AST validation, but that code is currently non-existant.
	*/
	constexpr auto TOP_STMT_TYPES = {
		StatementType::VARIABLE,
		//StatementType::IF, //conditional compilation
		StatementType::MODULE, StatementType::IMPORT,
		StatementType::FUNCTION,
		StatementType::SHADER,
		StatementType::STRUCT, StatementType::RECORD, StatementType::CLASS,
		//StatementType::ENUM,
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
		const StatementType type;

		StmtModifiers mods = {};
		std::map<std::string, uptr<Annotation>> annotations;

		Statement(StatementType stmtType) : type(stmtType) {}
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
		virtual void declareHeader(sptr<SymbolTable> table) {}

		virtual void declareSymbols(sptr<SymbolTable> table) = 0;

		virtual void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) = 0;

	};

	/*
	Defines a scope, which contains its own symbol table. Said table can shadow other symbols.
	*/
	struct ScopeStatement : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<Statement>> stmts;
		
		sptr<SymbolTable> scopeTable = nullptr;

		ReturnMode retMode = ReturnMode::NONE;
		sptr<Value> retValue = nullptr;

		ScopeStatement(StatementType stmtType = StatementType::SCOPE) : Statement(stmtType) {}
		virtual ~ScopeStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		void declareSymbols(sptr<SymbolTable> table) override;
		
		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

}
