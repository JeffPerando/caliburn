
#pragma once

#include <functional>

#include "buffer.h"
#include "compilererr.h"
#include "syntax.h"
#include "type.h"

#include "ast.h"

namespace caliburn
{
	class Parser;

	template<typename T>
	using ParseMethod = T* (Parser::*)();

	class Parser
	{
	public:
		Parser() {}
		virtual ~Parser() {}

		void parse(ptr<std::vector<Token>> tokenList, ptr<std::vector<ptr<Statement>>> ast);

	private:
		ptr<buffer<Token>> tokens = nullptr;
		std::vector<ptr<CaliburnException>> errors;

		template<typename T>
		ptr<T> parseAny(std::initializer_list<ParseMethod<T>> fns);

		template<typename T>
		ptr<T> parseBetween(std::string start, ParseMethod<T> fn, std::string end);

		void parseAnyBetween(std::string start, std::function<void()> fn, std::string end);

		void postParseException(CaliburnException* ex);

		void parseIdentifierList(ref<std::vector<ptr<Token>>> ids);

		bool parseGenerics(ref<std::vector<ptr<ParsedType>>> generics);

		bool parseValueList(ref<std::vector<ptr<Value>>> values, bool commaOptional);

		bool parseSemicolon();

		bool parseScopeEnd(ptr<ScopeStatement> stmt);

		ptr<ParsedType> parseTypeName();

		StorageModifiers parseStorageMods();

		ptr<ScopeStatement> parseScope(std::initializer_list<ParseMethod<Statement>> pms);

		ptr<Statement> parseDecl();
		
		ptr<Statement> parseImport();

		ptr<Statement> parseModuleDef();

		ptr<Statement> parseTypedef();

		//ptr<Statement> parseShader();

		ptr<Statement> parseStruct();

		//ptr<Statement> parseClass();
		
		ptr<Statement> parseFunction();

		ptr<Statement> parseMethod();

		ptr<Statement> parseConstructor();

		ptr<Statement> parseDestructor();

		//ptr<Statement> parseOp();

		ptr<Statement> parseLogic();

		//ptr<Statement> parseSetter();

		ptr<Statement> parseControl();

		ptr<Statement> parseIf();

		ptr<Statement> parseFor();
		
		ptr<Statement> parseWhile();
		
		ptr<Statement> parseDoWhile();

		ptr<Statement> parseValueStmt();

		ptr<Statement> parseMemberVarStmt();

		ptr<Value> parseAnyValue();

		ptr<Value> parseNonExpr();

		ptr<Value> parseLiteral();

		ptr<Value> parseAnyExpr();

		ptr<Value> parseExpr(uint32_t precedence);

		ptr<Value> parseAnyFnCall();

		ptr<Value> parseFnCall(ptr<Value> start);

		ptr<Variable> parseLocalVar();

		ptr<Variable> parseMemberVar();

	};

}
