
#include "parser.h"

#include "ctrlstmnt.h"
#include "miscstmt.h"
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

		ids.push_back(tkn.str);
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

Statement* Parser::parseAny(std::initializer_list<ParseMethod> fns)
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
		return tkn.str;
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
	type->name = tkn.str;

	parseGenerics(type->generics);

	return type;
}

bool Parser::parseSemicolon()
{
	//here because we need the line/column data
	Token tkn = tokens->current();

	if (tkn.identifier == CALIBURN_T_END)
	{
		tokens->consume();
		return true;
	}

	//TODO complain
	return false;
}

Statement* Parser::parseDecl()
{
	Statement* stmt = parseAny({
		&Parser::parseLogic,
		&Parser::parseFunction,
		&Parser::parseShader,
		&Parser::parseClass,
		&Parser::parseImport,
		&Parser::parseUsing,
		&Parser::parseTypedef,
		&Parser::parseNamespaceDef,
		&Parser::parseDescriptor,
		&Parser::parseStruct });

	if (!parseSemicolon())
	{
		//TODO complain
	}

	return stmt;
}

Statement* Parser::parseImport()
{
	if (tokens->current() != "import")
	{
		return nullptr;
	}

	Token importMod = tokens->next();
	tokens->consume();

	return new ImportStatement(importMod);
}

Statement* Parser::parseUsing()
{
	if (tokens->current() != "using")
	{
		return nullptr;
	}

	Token usedNamespace = tokens->next();
	tokens->consume();

	return new UsingStatement(usedNamespace);
}

Statement* Parser::parseTypedef()
{
	if (tokens->current() != "type")
	{
		return nullptr;
	}

	if (tokens->next().identifier != CALIBURN_T_IDENTIFIER)
	{
		return nullptr;
	}

	std::string name = tokens->current();

	if (tokens->next() != "=")
	{
		return nullptr;
	}

	tokens->consume();

	ParsedType* aliasedType = parseTypeName();

	auto stmt = new TypedefStatement();
	
	stmt->name = name;
	stmt->actualType = aliasedType;

	return stmt;
}

//TODO more parser methods

/*
Statement* parseNamespaceDef();

Statement* parseShader();

Statement* parseDescriptor();

Statement* parseStruct();

Statement* parseClass();
*/

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

	if (tkn.str != "if")
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

	if (tokens->current().str == "else")
	{
		tokens->consume();
		parsed->elseBranch = parseLogic();

	}

	return parsed;
}

Statement* Parser::parseFor()
{
	if (tokens->current() != "for")
	{
		return nullptr;
	}

	if (tokens->next().identifier != CALIBURN_T_START_PAREN)
	{
		return nullptr;
	}

	tokens->consume();

	Statement* pre = parseLogic();
	parseSemicolon();

	ValueStatement* cond = (ValueStatement*)parseValue();
	parseSemicolon();

	Statement* post = parseLogic();

	if (tokens->current().identifier != CALIBURN_T_END_PAREN)
	{
		return nullptr;
	}

	tokens->consume();

	Statement* loop = parseLogic();

	ForStatement* stmt = new ForStatement();

	stmt->preLoop = pre;
	stmt->cond = cond;
	stmt->postLoop = post;
	stmt->loop = loop;

	return stmt;
}

Statement* Parser::parseFunction()
{
	Token tkn = tokens->current();

	if (tkn.str != "def")
	{
		return nullptr;
	}

	tkn = tokens->next();

	//parse optional visibility
	Visibility vis = strToVis(tkn.str);

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
	if (tkn.str == "[")
	{
		parseIdentifierList(gpuThreadData);

		if (gpuThreadData.size() == 0)
		{
			//TODO complain
		}

		if (tokens->current().str != "]")
		{
			//TODO also complain
		}

		tkn = tokens->next();

	}

	if (tkn.str != "(")
	{
		return nullptr;
	}

	//TODO parse arg list

	tkn = tokens->next();

	if (tkn.str != ")")
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
		if (tkn.str == "let" && implicitAllowed)
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

	if (tokens->current().str == "=")
	{
		tokens->consume();
		defVal = (ValueStatement*)parseValue();
		//type = defVal->type;

	}
	else if (!type)
	{
		//TODO complain
		return nullptr;
	}

}
