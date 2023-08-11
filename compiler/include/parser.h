
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
	struct StructStatement;
	struct Parser;

	template<typename T>
	using ParseMethod = std::function<T(Parser&)>;
	
	struct Parser
	{
	private:
		Buffer<sptr<Token>> tokens;
	public:
		const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::PARSER);

		Parser(std::vector<sptr<Token>> tokenVec) : tokens(tokenVec) {}
		virtual ~Parser() {}

		std::vector<uptr<Statement>> parse();

		template<typename T>
		T parseAny(std::vector<ParseMethod<T>> fns);

		template<typename T>
		T parseBetween(std::string start, ParseMethod<T> fn, std::string end);

		void parseAnyBetween(std::string start, std::function<void()> fn, std::string end);

		std::vector<sptr<Token>> parseIdentifierList();

		sptr<GenericSignature> parseGenericSig();

		sptr<GenericArguments> parseGenericArgs();

		bool parseValueList(ref<std::vector<sptr<Value>>> values, bool commaOptional);

		bool parseSemicolon();

		bool parseScopeEnd(ref<uptr<ScopeStatement>> stmt);

		sptr<ParsedType> parseTypeName();

		StmtModifiers parseStmtMods();

		uptr<ScopeStatement> parseScope(std::vector<ParseMethod<uptr<Statement>>> pms);

		uptr<Statement> parseDecl();
		
		uptr<Statement> parseImport();

		uptr<Statement> parseModuleDef();

		uptr<Statement> parseTypedef();

		uptr<Statement> parseShader();

		uptr<Statement> parseStruct();

		uptr<Statement> parseFnStmt();

		uptr<ParsedFn> parseFnLike();

		sptr<Function> parseMethod();

		uptr<Statement> parseLogic();

		uptr<Statement> parseControl();

		uptr<Statement> parseIf();

		uptr<Statement> parseFor();
		
		uptr<Statement> parseWhile();
		
		uptr<Statement> parseDoWhile();

		uptr<Statement> parseValueStmt();

		uptr<Statement> parseGlobalVarStmt();

		uptr<Statement> parseLocalVarStmt();

		sptr<Value> parseAnyValue();

		sptr<Value> parseNonExpr();

		sptr<Value> parseLiteral();

		sptr<Value> parseAnyAccess();

		sptr<Value> parseAccess(sptr<Value> target);

		sptr<Value> parseAnyExpr();

		sptr<Value> parseExpr(uint32_t precedence);

		sptr<Value> parseAnyFnCall();

		sptr<Value> parseFnCall(sptr<Value> start);

		std::vector<sptr<Variable>> parseLocalVars();

		sptr<Variable> parseGlobalVar();

		sptr<Variable> parseMemberVar();

		std::vector<sptr<ParsedVar>> parseLocalVarLike();

		std::vector<sptr<ParsedVar>> parseMemberVarLike();

		std::vector<uptr<ParsedFnArg>> parseFnArgs();

		std::vector<uptr<Annotation>> parseAllAnnotations();

		uptr<Annotation> parseAnnotation();

	};

}
