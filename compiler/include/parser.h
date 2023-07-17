
#pragma once

#include <functional>
#include <utility>

#include "buffer.h"
#include "error.h"
#include "syntax.h"

#include "ast/ast.h"
#include "ast/var.h"

#include "types/type.h"

namespace caliburn
{
	struct GenericArguments;
	struct GenericSignature;
	struct StructStatement;
	struct Parser;

	template<typename T>
	using ParseMethod = std::function<T(Parser&)>;
	
	struct Parser
	{
		const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::PARSER);

		Parser(std::vector<sptr<Token>> tokenVec) : tokens(tokenVec) {}
		virtual ~Parser() {}

		std::vector<uptr<Statement>> parse();

	private:
		Buffer<sptr<Token>> tokens;
		
		template<typename T>
		uptr<T> parseAnyUnique(std::vector<ParseMethod<uptr<T>>> fns);

		template<typename T>
		sptr<T> parseAnyShared(std::vector<ParseMethod<sptr<T>>> fns);

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

		sptr<Function> parseFunction();

		std::pair<sptr<Token>, Member> parseMember(bool isConst = false);

		sptr<Function> parseMethod();

		sptr<Function> parseConstructor();

		sptr<Function> parseDestructor();

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

		sptr<Value> parseAnyExpr();

		sptr<Value> parseExpr(uint32_t precedence);

		sptr<Value> parseAnyFnCall();

		sptr<Value> parseFnCall(sptr<Value> start);

		bool parseLocalVars(ref<std::vector<sptr<Variable>>> vars);

		sptr<Variable> parseGlobalVar();

		sptr<Variable> parseMemberVar();

		std::vector<sptr<FnArgVariable>> parseFnArgs();

		std::vector<uptr<Annotation>> parseAllAnnotations();

		uptr<Annotation> parseAnnotation();

	};

}
