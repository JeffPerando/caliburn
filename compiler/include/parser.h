
#pragma once

#include <functional>
#include <utility>

#include "buffer.h"
#include "error.h"
#include "syntax.h"

#include "ast/ast.h"
#include "ast/fn.h"
#include "ast/generics.h"
#include "ast/var.h"

namespace caliburn
{
	struct Parser;

	template<typename T>
	using ParseMethod = std::function<T(Parser&)>;
	
	/*
	The Caliburn parser is a hand-rolled parser with the hardest job of all stages: Interpreting user code into a valid AST.

	The principles it abides by are:
	1. All methods must either consume no tokens, or consume all needed tokens.
	2. All methods must end on the first invalid token.
	3. Buffer overflows are forbidden; If there's a minimum number of tokens, then that should be checked first.
	4. If parsing fails early, then return null.
	*/
	struct Parser
	{
	private:
		sptr<const CompilerSettings> settings;
		Buffer<sptr<Token>> tkns;
	public:
		const uptr<ErrorHandler> errors;

		Parser(sptr<const CompilerSettings> cs, in<std::vector<sptr<Token>>> tokenVec) :
			settings(cs), tkns(tokenVec), errors(new_uptr<ErrorHandler>(CompileStage::PARSER, cs)) {}
		virtual ~Parser() {}

		/*
		Goes through the token buffer until it hits a semicolon.

		If a scope is encountered, then the scope, and all child scopes and scoped semicolons, will be skipped.
		So the code {;;;}; will find the semicolon at the end of scope, ignoring the 3 within the scope.

		This is used to try and salvage silly code. Such code should still error out, though.
		*/
		void skipStmt();

		/*
		Initial AST-generating method.
		*/
		std::vector<uptr<Statement>> parse();

		/*
		Iterates through the list of functions until it finds a non-null result, then returns it.
		
		If parsing fails, then the buffer index is reset to the one it was on when the method was called.
		*/
		template<typename T>
		T parseAny(in<std::vector<ParseMethod<T>>> fns);

		/*
		Checks the current token for the starting token, then invokes the function, then checks for the end token.

		If either start or end tokens don't match, this method will return false.

		If parsing is successful, it will return true.
		*/
		bool parseAnyBetween(in<std::string> start, in<std::function<void()>> fn, in<std::string> end);

		/*
		Finds a list of comma-separated identifiers.
		*/
		std::vector<sptr<Token>> parseIdentifierList();

		/*
		Parses a generic signature.

		Generally, generic signatures go along with type or function definitions.
		*/
		uptr<GenericSignature> parseGenericSig();

		/*
		Parses generic arguments. This tends to go with function calls or a type definition.
		*/
		sptr<GenericArguments> parseGenericArgs();

		/*
		Parses out values, with optional comma-separation.

		Commas are optional in array literals, and that's about it.
		*/
		std::vector<sptr<Value>> parseValueList(bool commaOptional);

		/*
		Looks for an ending semicolon. If it's not found, an error is produced.
		*/
		void parseSemicolon();

		/*
		Looks for an end-of-scope statement, like return, break, pass, etc.

		Returns true if one was found, false otherwise.

		The results are stored inside the passed scope statement.
		*/
		bool parseScopeEnd(out<ScopeStatement> stmt);

		/*
		Parses a type name.

		Generally it's only a good idea to use this when one is expected, since there can be overlap with construction.
		*/
		sptr<ParsedType> parseTypeName();

		/*
		Parses statement modifiers.

		Failure to find a modifier does nothing.
		*/
		StmtModifiers parseStmtMods();

		/*
		Parses a scope with the given parse methods.

		Hypothetically there are contexts where errors are unneeded, hence the error flag.
		*/
		uptr<ScopeStatement> parseScope(in<std::vector<ParseMethod<uptr<Statement>>>> pms, bool err = true);

		/*
		Parses a top-level declaration according to the Caliburn standard.
		*/
		uptr<Statement> parseDecl();
		
		/*
		Parses an import statement

		Examples:
		
		import math;
		import calburn as cbrn;
		*/
		uptr<Statement> parseImport();

		/*
		Parses a module-defining statement. This names the current module being compiled.

		Example:

		module caliburn;
		module MyShaders;
		*/
		uptr<Statement> parseModuleDef();

		/*
		Parses a type-aliasing statement.

		Examples:

		type x = y;
		type FP = dynamic<float32>;
		*/
		uptr<Statement> parseTypedef();

		/*
		Parses a shader object statement
		*/
		uptr<Statement> parseShader();

		/*
		Parses a struct, class, or record statement
		*/
		uptr<Statement> parseStruct();

		/*
		Parses a standalone function statement
		*/
		uptr<Statement> parseFnStmt();

		/*
		Parses a function-like statement; These include:
		* Functions
		* Methods
		* Constructors
		* Destructors
		* Operator overrides
		* Implicit conversions
		*/
		uptr<ParsedFn> parseFnLike();

		/*
		Parses a method; This uses parseFnLike() to make a proper Function object
		*/
		sptr<Function> parseMethod();

		/*
		Parses logical statements; These are generally control statements, setters, and function calls.
		*/
		uptr<Statement> parseLogic();

		/*
		Parses control-flow statements. Things like if, for, and while.
		*/
		uptr<Statement> parseControl();

		/*
		Parses an if statement.
		*/
		uptr<Statement> parseIf();

		/*
		Parses a for statement.

		Currently not implemented; Currently awaiting standard finalization, or at least clarification.
		*/
		uptr<Statement> parseFor();
		
		/*
		Parses a while statement.

		Unlike the for statement, this one is available now.
		*/
		uptr<Statement> parseWhile();
		
		/*
		Parses a do/while statement.

		Probably not a good idea, but eh
		*/
		uptr<Statement> parseDoWhile();

		/*
		A value statement is either a setter or function call.

		Why a "value statement"? Because both are covered by value-parsing code and I didn't want to repeat it.
		*/
		uptr<Statement> parseValueStmt();

		/*
		Parses a global variable statement.

		These are constant-only variables, and thus the parsing code is a little simpler than the code for local variables.
		*/
		uptr<Statement> parseGlobalVarStmt();

		/*
		Parses a local variable statement.
		*/
		uptr<Statement> parseLocalVarStmt();

		/*
		Parses math expressions. This is the primary method for reading values.
		*/
		sptr<Value> parseExpr();

		/*
		Parses high-precedent values which can be terms in a math expression.
		*/
		sptr<Value> parseTerm();

		/*
		Parses all forms of literals, including int, float, bool, string, array, and keyword values.
		*/
		sptr<Value> parseLiteral();

		/*
		Parses any values preceded by a unary operator.
		*/
		sptr<Value> parseUnaryValue();

		/*
		Parses values starting with parentheses.
		*/
		sptr<Value> parseParenValue();

		/*
		Parses variable reads, as well as member access.
		*/
		sptr<Value> parseAnyAccess();

		/*
		Internal method for parsing member accesses. Also checks for function/method calls
		*/
		sptr<Value> parseAccess(sptr<Value> target);

		/*
		Parses function calls; includes generic arguments
		*/
		sptr<Value> parseAnyFnCall();

		/*
		Parses method calls
		*/
		sptr<Value> parseFnCall(sptr<Value> start);

		/*
		Parses local variables; Used by parseLocalVarStmt mainly
		*/
		std::vector<sptr<Variable>> parseLocalVars();

		/*
		Parses local variables; Used by parseGlobalVarStmt mainly
		*/
		sptr<Variable> parseGlobalVar();

		/*
		Parses member variables; Used by parseStruct
		*/
		sptr<Variable> parseMemberVar();

		/*
		Parses local var-likes; these use the rough form of:

		("var" || "const") (":" <type>)? <name>

		such as:
		var x;
		const: int i = 0;

		*/
		std::vector<sptr<ParsedVar>> parseLocalVarLike();

		/*
		Parses member variable-likes; these use the rough form of:
		
		<type> <name>

		e.g. int x;

		They can also optionally have an initial value.
		*/
		std::vector<sptr<ParsedVar>> parseMemberVarLike();

		/*
		Parses function arguments; These have an identical structure to member var-likes, but have a much simpler output type
		*/
		std::vector<uptr<ParsedFnArg>> parseFnArgs();

		/*
		This method parses all of the annotations preceding a statement.
		*/
		std::vector<uptr<Annotation>> parseAllAnnotations();

		/*
		Parses a single annotation. See the Annotation struct for more info.
		*/
		uptr<Annotation> parseAnnotation();

	};

}
