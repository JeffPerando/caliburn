
#pragma once

#include "buffer.h"
#include "compilererr.h"
#include "syntax.h"
#include "type.h"

#include "ctrlstmnt.h"

namespace caliburn
{
	class Parser;

	using ParseMethod = Statement* (Parser::*)(Statement*);

	class Parser
	{
		buffer<Token>* tokens = nullptr;
		std::vector<CaliburnException*> errors;

		void postParseException(CaliburnException* ex);

		void parseIdentifierList(std::vector<std::string>& ids);

		void parseValueList(std::vector<ValueStatement*>& xs);
		
		bool parseGenerics(std::vector<ParsedType*>& generics);

		bool parseArrayList(std::vector<ValueStatement*>& xs);

		Statement* parseAny(std::initializer_list<ParseMethod> fns);

		Token* parseNamespace();

		ParsedType* parseTypeName();

		bool parseSemicolon();

		Statement* parseDecl();
		
		Statement* parseImport();

		Statement* parseTypedef();
		
		//Statement* parseNamespaceDef();

		//Statement* parseShader();

		//Statement* parseDescriptor();

		//Statement* parseStruct();

		//Statement* parseClass();
		
		Statement* parseFunction();

		//Statement* parseMethod();

		//called a "statement" in the CFG
		//variable, setter, control flow, scope, or function call
		Statement* parseLogic();

		Statement* parseAnyVar();

		Statement* parseVariable(bool implicitAllowed = true);

		//Statement* parseSetter();

		Statement* parseScope();

		Statement* parseControl();

		Statement* parseIf();

		Statement* parseFor();
		
		Statement* parseWhile();
		
		Statement* parseDoWhile();

		Statement* parseBreak();

		Statement* parseContinue();
		
		Statement* parseSwitch();
		
		Statement* parseCase();

		Statement* parsePass();
		
		Statement* parseReturn();
		
		Statement* parseStmtInParentheses(ParseMethod pm);

		//Statement* parseLiteral();

	public:
		Parser() {}
		~Parser() {}

		void parse(std::vector<Token>* tokenList, std::vector<Statement*>* ast);

	};

}
