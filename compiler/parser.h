
#pragma once

#include <functional>

#include "ast.h"
#include "buffer.h"
#include "compilererr.h"
#include "generics.h"
#include "syntax.h"
#include "type.h"

namespace caliburn
{
	class Parser;

	template<typename T>
	using ParseMethod = uptr<T> (Parser::*)();

	class Parser
	{
	public:
		Parser() {}
		virtual ~Parser() {}

		void parse(ref<std::vector<sptr<Token>>> tokenList, ref<std::vector<uptr<Statement>>> ast);

	private:
		ptr<buffer<sptr<Token>>> tokens = nullptr;
		std::vector<uptr<CaliburnException>> errors;

		template<typename T>
		uptr<T> parseAny(std::initializer_list<ParseMethod<T>> fns);

		template<typename T>
		uptr<T> parseBetween(std::string start, ParseMethod<T> fn, std::string end);

		void parseAnyBetween(std::string start, std::function<void()> fn, std::string end);

		void postParseException(uptr<CaliburnException> ex);

		void parseIdentifierList(ref<std::vector<sptr<Token>>> ids);

		bool parseGenericSig(ref<GenericSignature> sig);

		bool parseGenericArgs(ref<GenericArguments> args);

		bool parseValueList(ref<std::vector<uptr<Value>>> values, bool commaOptional);

		bool parseSemicolon();

		bool parseScopeEnd(uptr<ScopeStatement> stmt);

		uptr<ParsedType> parseTypeName();

		StorageModifiers parseStorageMods();

		uptr<ScopeStatement> parseScope(std::initializer_list<ParseMethod<Statement>> pms);

		uptr<Statement> parseDecl();
		
		uptr<Statement> parseImport();

		uptr<Statement> parseModuleDef();

		uptr<Statement> parseTypedef();

		uptr<Statement> parseShader();

		uptr<Statement> parseStruct();

		//uptr<Statement> parseClass();
		
		uptr<Statement> parseFunction();

		uptr<Statement> parseMethod();

		uptr<Statement> parseConstructor();

		uptr<Statement> parseDestructor();

		//uptr<Statement> parseOp();

		uptr<Statement> parseLogic();

		//uptr<Statement> parseSetter();

		uptr<Statement> parseControl();

		uptr<Statement> parseIf();

		uptr<Statement> parseFor();
		
		uptr<Statement> parseWhile();
		
		uptr<Statement> parseDoWhile();

		uptr<Statement> parseValueStmt();

		uptr<Statement> parseLocalVarStmt();

		uptr<Value> parseAnyValue();

		uptr<Value> parseNonExpr();

		uptr<Value> parseLiteral();

		uptr<Value> parseAnyExpr();

		uptr<Value> parseExpr(uint32_t precedence);

		uptr<Value> parseAnyFnCall();

		uptr<Value> parseFnCall(uptr<Value> start);

		sptr<Variable> parseMemberVar();

	};

}
