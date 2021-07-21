
#include "parser.h"

#include "ctrlstmnt.h"
#include "funcstmnt.h"
#include "scopestmnt.h"

//I'm so sorry
using namespace caliburn;

void Parser::parse(std::vector<Token>* tokenList, std::vector<Statement*>* ast)
{
	//don't want to pollute the heap too much
	auto bufferTmp = buffer<Token>(tokenList);
	tokens = &bufferTmp;

	while (tokens->hasNext())
	{
		Statement* finished = parseDecl();

		if (finished)
		{
			ast->push_back(finished);
		}
		//TODO implement error system
		else
		{
			break;
		}

	}

}

void Parser::parseIdentifierList(std::vector<std::string>& ids)
{
	while (true)
	{
		Token tkn = tokens->current();

		if (tkn.identifier != CALIBURN_T_IDENTIFIER)
		{
			break;
		}

		ids.push_back(tkn.token);
		tkn = tokens->next();

		if (tkn.identifier == CALIBURN_T_COMMA)
		{
			tokens->consume();
			continue;
		}

	}

}

void Parser::parseGenerics(std::vector<ParsedType*>& generics)
{
	if (tokens->current().identifier == CALIBURN_T_LT_SIGN)
	{
		tokens->consume();

		while (true)
		{
			generics.push_back(parseTypeName());

			Token tkn = tokens->current();

			if (tkn.identifier == CALIBURN_T_GT_SIGN)
			{
				tokens->consume();
				break;
			}
			else if (tkn.identifier == CALIBURN_T_COMMA)
			{
				continue;
			}

			//TODO complain
		}

	}
}

Statement* Parser::parseAny(std::initializer_list<caliburn::Statement* (caliburn::Parser::*)()> fns)
{
	/*
	this code is really smart, and stupid. at the same time.
	it takes a bunch of method/function pointers, puts them into a list,
	then calls them all. if it finds one that doesn't return nullptr,
	it returns that result.
	The alternative was very repetitive code.
	*/
	auto current = tokens->currentIndex();

	for (auto fn : fns)
	{
		Statement* parsed = (this->*fn)();

		if (parsed)
		{
			return parsed;
		}
		else
		{
			//undo any funny business the parse function may have done
			tokens->revertTo(current);

		}

	}

	return nullptr;
}

std::string Parser::parseNamespace()
{
	if (tokens->current().identifier == CALIBURN_T_IDENTIFIER &&
		tokens->peek().identifier == CALIBURN_T_COLON)
	{
		Token tkn = tokens->current();
		tokens->consume(2);
		return tkn.token;
	}

	return "";
}

ParsedType* Parser::parseTypeName()
{
	std::string moduleName = parseNamespace();

	Token tkn = tokens->current();

	if (tkn.identifier != CALIBURN_T_IDENTIFIER)
	{
		//TODO complain?
		return nullptr;
	}

	tokens->consume();

	ParsedType* type = new ParsedType();

	type->mod = moduleName;
	type->name = tkn.token;

	parseGenerics(type->generics);

	return type;
}

Statement* Parser::parseDecl()
{
	return parseAny({
		&Parser::parseLogic,
		&Parser::parseFunction,
		&Parser::parseShader,
		&Parser::parseClass,
		&Parser::parseImport,
		&Parser::parseUsing,
		&Parser::parseTypedef,
		&Parser::parseModule,
		&Parser::parseDescriptor,
		&Parser::parseStruct });
}

Statement* Parser::parseImport()
{
	Token tkn = tokens->current();

	if (tkn.token != "import")
	{
		return nullptr;
	}

	tokens->consume();

	//TODO create import statement
	return nullptr;
}

//TODO more parser methods

Statement* Parser::parseScope()
{
	Token tkn = tokens->current();

	if (tkn.identifier != CALIBURN_T_START_SCOPE)
	{
		return nullptr;
	}

	tokens->consume();

	ScopeStatement* scope = new ScopeStatement();

	while (tokens->current().identifier != CALIBURN_T_END_SCOPE)
	{
		scope->innerCode.push_back(parseDecl());
		
	}

	tokens->consume();

	return scope;
}

Statement* Parser::parseLogic()
{
	return parseAny({ &Parser::parseControl, &Parser::parseScope,
		&Parser::parseAnyVar, &Parser::parseSetter });
}

Statement* Parser::parseControl()
{
	Token tkn = tokens->current();

	if (tkn.identifier != CALIBURN_T_KEYWORD)
	{
		return nullptr;
	}

	return parseAny({ &Parser::parseIf, &Parser::parseFor, &Parser::parseWhile,
		&Parser::parseDoWhile, &Parser::parseSwitch });
}

Statement* Parser::parseIf()
{
	Token tkn = tokens->current();

	if (tkn.token != "if")
	{
		return nullptr;
	}

	if (tokens->peek().identifier != CALIBURN_T_START_PAREN)
	{
		return nullptr;
	}

	tokens->consume();

	ValueStatement* condition = (ValueStatement*)parseValue();

	if (tokens->current().identifier != CALIBURN_T_END_PAREN)
	{
		//BIG OOF
		delete condition;
		return nullptr;
	}

	tokens->consume();

	IfStatement* parsed = new IfStatement();

	parsed->condition = condition;
	parsed->ifBranch = parseLogic();

	if (tokens->current().token == "else")
	{
		tokens->consume();
		parsed->elseBranch = parseLogic();

	}

	return parsed;
}

Statement* Parser::parseFunction()
{
	Token tkn = tokens->current();

	if (tkn.token != "def")
	{
		return nullptr;
	}

	tkn = tokens->next();

	//parse optional visibility
	Visibility vis = strToVis(tkn.token);

	if (vis != Visibility::NONE)
	{
		tkn = tokens->next();

	}

	ParsedType* type = parseTypeName();
	std::string name = tokens->current().str;
	std::vector<ParsedType*> generics;
	std::vector<std::string> gpuThreadData;
	
	tokens->consume();

	parseGenerics(generics);

	//parse GPU threading data
	if (tkn.token == "[")
	{
		parseIdentifierList(gpuThreadData);

		if (gpuThreadData.size() == 0)
		{
			//TODO complain
		}

		if (tokens->current().token != "]")
		{
			//TODO also complain
		}

		tkn = tokens->next();

	}

	if (tkn.token != "(")
	{
		return nullptr;
	}

	//TODO parse arg list

	tkn = tokens->next();

	if (tkn.token != ")")
	{
		return nullptr;
	}

	Statement* body = parseLogic();

	if (!body)
	{
		return nullptr;
	}

	FunctionStatement* func = new FunctionStatement();

	//func->type = type;
	func->name = name;
	func->funcBody = body;
	//TODO finish

	return func;
}

Statement* Parser::parseAnyVar()
{
	return parseVariable(true);
}

Statement* Parser::parseVariable(bool implicitAllowed)
{
	ParsedType* type = nullptr;
	std::vector<std::string> vars;
	ValueStatement* defVal = nullptr;

	Token tkn = tokens->current();

	if (tkn.identifier == CALIBURN_T_KEYWORD)
	{
		if (tkn.token == "let" && implicitAllowed)
		{
			tokens->consume();

		}
		else
		{
			//TODO complain
			return nullptr;
		}

	}
	else
	{
		type = parseTypeName();

	}

	parseIdentifierList(vars);

	if (vars.size() == 0)
	{
		//TODO complain
		return nullptr;
	}

	if (tokens->current().token == "=")
	{
		tokens->consume();
		defVal = (ValueStatement*)parseValue();

	}

}
