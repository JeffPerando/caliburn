
#pragma once

#include "buffer.h"
#include "statement.h"
#include "tokenizer.h"
#include "type.h"

namespace caliburn
{
	class Parser
	{
	private:
		buffer<Token>* tokens = nullptr;

		void parseIdentifierList(std::vector<std::string>& ids);

		void parseGenerics(std::vector<ParsedType*>& generics);

		Statement* parseAny(std::initializer_list<ParseMethod> fns);

		std::string parseNamespace();

		ParsedType* parseTypeName();

		Statement* parseDecl();
		
		Statement* parseImport();

		Statement* parseUsing();

		Statement* parseTypedef();

		Statement* parseModule();

		Statement* parseShader();

		Statement* parseDescriptor();

		Statement* parseStruct();

		Statement* parseClass();

		Statement* parseFunction();

		Statement* parseMethod();

		Statement* parseAnyVar();

		Statement* parseVariable(bool implicitAllowed = true);

		Statement* parseSetter();

		Statement* parseScope();

		//called a "statement" in the CFG
		//variable, setter, control flow, scope, or function call
		Statement* parseLogic();

		Statement* parseControl();

		Statement* parseIf();

		Statement* parseFor();

		Statement* parseWhile();

		Statement* parseDoWhile();

		Statement* parseSwitch();

		Statement* parseBreak();

		Statement* parseContinue();

		Statement* parsePass();

		Statement* parseReturn();

		Statement* parseValue();

		Statement* parseLValue();

		Statement* parseRValue();

		Statement* parseLiteral();

	public:
		Parser() {}
		~Parser() {}

		void parse(std::vector<Token>* tokenList, std::vector<Statement*>* ast);

	};

	using ParseMethod = Statement * (Parser::*)();

}
