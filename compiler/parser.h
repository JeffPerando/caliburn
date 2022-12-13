
#pragma once

#include "buffer.h"
#include "compilererr.h"
#include "syntax.h"
#include "type.h"

#include "ast.h"
#include "scopestmt.h"
//#include "ctrlstmnt.h"

namespace caliburn
{
	class Parser;

	using ParseMethod = Statement* (Parser::*)(Statement*);

	class Parser
	{
	public:
		Parser() {}
		~Parser() {}

		void parse(std::vector<Token>* tokenList, std::vector<Statement*>* ast);

	private:
		buffer<Token>* tokens = nullptr;
		std::vector<CaliburnException*> errors;

		void postParseException(CaliburnException* ex);

		void parseIdentifierList(std::vector<std::string>& ids);

		void parseValueList(std::vector<Value*>& xs);
		
		bool parseGenerics(std::vector<ParsedType*>& generics);

		bool parseArrayList(std::vector<Value*>& xs);

		bool parseSemicolon();

		bool parseScopeEnd(Statement* stmt);

		Token* parseNamespace();

		ParsedType* parseTypeName();

		StorageModifiers parseStorageMods();

		Statement* parseAny(Statement* parent, std::initializer_list<ParseMethod> fns);

		Statement* parseDecl(Statement* parent);
		
		//Statement* parseImport(Statement* parent);

		//Statement* parseTypedef(Statement* parent);
		
		//Statement* parseNamespaceDef(Statement* parent);

		//Statement* parseShader(Statement* parent);

		//Statement* parseStruct(Statement* parent);

		//Statement* parseClass(Statement* parent);
		
		Statement* parseFunction(Statement* parent);

		//Statement* parseMethod(Statement* parent);

		Statement* parseLogic(Statement* parent);

		//Statement* parseSetter(Statement* parent);

		Statement* parseScope(Statement* parent);

		Statement* parseControl(Statement* parent);

		Statement* parseIf(Statement* parent);

		Statement* parseFor(Statement* parent);
		
		Statement* parseWhile(Statement* parent);
		
		Statement* parseDoWhile(Statement* parent);

		Statement* parseSwitch(Statement* parent);
		
		Statement* parseCase(Statement* parent);
		
		//Statement* parseStmtInParentheses(ParseMethod pm);
		
		Value* parseValue(bool doPostfix = true);

		//Value* parseLiteral();
		
	};

}
