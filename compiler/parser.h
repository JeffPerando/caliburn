
#pragma once

#include "buffer.h"
#include "statement.h"
#include "tokenizer.h"

namespace caliburn
{
	class Parser
	{
	private:
		buffer<Token>* tokens = nullptr;

		Statement* parseAny(std::initializer_list<caliburn::Statement* (caliburn::Parser::*)()> fns);

		void parseIdentifierList(std::vector<std::string>& ids);

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

		Statement* parseVariable();

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

		std::vector<Statement*>* parse(std::vector<Token>* tokenList);

	};

	void parse(buffer<Token>* tokens, std::vector<Statement*>* ast);

}
