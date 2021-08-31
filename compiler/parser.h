
#pragma once

#include "buffer.h"
#include "compilererr.h"
#include "statement.h"
#include "tokenizer.h"
#include "type.h"

#include "ctrlstmnt.h"
#include "funcstmnt.h"
#include "miscstmt.h"
#include "objstmt.h"
#include "scopestmnt.h"

namespace caliburn
{
	class Parser;

	using ParseMethod = Statement * (Parser::*)();

	class Parser
	{
		buffer<Token>* tokens = nullptr;
		std::vector<CaliburnException*> errors;

		void postParseException(CaliburnException* ex);

		void parseIdentifierList(std::vector<std::string>& ids);

		void parseValueList(std::vector<Statement*>& xs);
		
		bool parseGenerics(std::vector<ParsedType*>& generics);

		Statement* parseAny(std::initializer_list<ParseMethod> fns);

		Token* parseNamespace();

		ParsedType* parseTypeName();

		bool parseSemicolon();

		Statement* parseDecl();
		
		Statement* parseImport();

		Statement* parseTypedef();
		/*
		Statement* parseNamespaceDef();

		Statement* parseShader();

		Statement* parseDescriptor();

		Statement* parseStruct();

		Statement* parseClass();
		*/
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
		/*
		Statement* parseDoWhile();

		Statement* parseSwitch();

		Statement* parseBreak();

		Statement* parseContinue();

		Statement* parsePass();

		Statement* parseReturn();
		*/
		Statement* parseValue();

		Statement* parseValue(bool doPostfix);

		Statement* parseAnyFieldOrFuncValue();

		Statement* parseFieldOrFuncValue(bool canHaveNamespace);

		//Statement* parseLiteral();

	public:
		Parser() {}
		~Parser() {}

		void parse(std::vector<Token>* tokenList, std::vector<Statement*>* ast);

	};

}
