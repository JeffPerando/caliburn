
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

		if (tkn.type != CALIBURN_T_IDENTIFIER)
		{
			break;
		}

		ids.push_back(tkn.str);
		tkn = tokens->next();

		if (tkn.type != CALIBURN_T_COMMA)
		{
			break;
		}

		tokens->consume();

	}

}

void Parser::parseValueList(std::vector<Statement*>& xs)
{
	while (true)
	{
		Statement* val = parseValue();

		if (val == nullptr)
		{
			break;
		}

		xs.push_back(val);
		
		if (tokens->current().type != CALIBURN_T_COMMA)
		{
			break;
		}

		tokens->consume();

	}

}

bool Parser::parseGenerics(std::vector<ParsedType*>& generics)
{
	bool actualGeneric = false;
	auto oldIndex = tokens->currentIndex();

	if (tokens->current().type == CALIBURN_T_LT_SIGN)
	{
		tokens->consume();

		actualGeneric = true;

		while (true)
		{
			auto type = parseTypeName();

			if (!type)
			{
				actualGeneric = false;
				break;
			}

			generics.push_back(type);

			Token tkn = tokens->current();

			if (tkn.type == CALIBURN_T_GT_SIGN)
			{
				tokens->consume();
				break;
			}
			else if (tkn.type == CALIBURN_T_COMMA)
			{
				tokens->consume();
				continue;
			}
			else
			{
				actualGeneric = false;
				break;
			}

		}

	}

	if (!actualGeneric)
	{
		for (auto type : generics)
		{
			delete type;
		}

		generics.clear();
		tokens->revertTo(oldIndex);

	}

	return actualGeneric;
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
	if (tokens->current().type == CALIBURN_T_IDENTIFIER &&
		tokens->peek().type == CALIBURN_T_COLON)
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

	if (tkn.type != CALIBURN_T_IDENTIFIER)
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

	if (tkn.type == CALIBURN_T_END)
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

	if (tokens->next().type != CALIBURN_T_IDENTIFIER)
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
Statement* Parser::parseNamespaceDef();

Statement* Parser::parseShader();

Statement* Parser::parseDescriptor();

Statement* Parser::parseStruct();

Statement* Parser::parseClass();
*/

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

//Statement* Parser::parseMethod();

Statement* Parser::parseLogic()
{
	return parseAny({ &Parser::parseControl, &Parser::parseScope,
		&Parser::parseAnyVar, &Parser::parseSetter });
}

Statement* Parser::parseFuncCall()
{
	Token tkn = tokens->current();

	if (tkn.type == CALIBURN_T_KEYWORD)
	{
		if (tkn != "dispatch")
		{
			return nullptr;
		}
	}
	else if (tkn.type != CALIBURN_T_IDENTIFIER)
	{
		return nullptr;
	}

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

	if (tkn.type == CALIBURN_T_KEYWORD)
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

//Statement* Parser::parseSetter();

Statement* Parser::parseScope()
{
	Token tkn = tokens->current();

	if (tkn.type != CALIBURN_T_START_SCOPE)
	{
		return nullptr;
	}

	tokens->consume();

	ScopeStatement* scope = new ScopeStatement();

	while (tokens->current().type != CALIBURN_T_END_SCOPE)
	{
		scope->innerCode.push_back(parseDecl());
		
	}

	tokens->consume();

	return scope;
}

Statement* Parser::parseControl()
{
	Token tkn = tokens->current();

	if (tkn.type != CALIBURN_T_KEYWORD)
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

	if (tokens->peek().type != CALIBURN_T_START_PAREN)
	{
		return nullptr;
	}

	tokens->consume();

	ValueStatement* condition = (ValueStatement*)parseValue();

	if (tokens->current().type != CALIBURN_T_END_PAREN)
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

	if (tokens->next().type != CALIBURN_T_START_PAREN)
	{
		return nullptr;
	}

	tokens->consume();

	Statement* pre = parseLogic();
	parseSemicolon();

	ValueStatement* cond = (ValueStatement*)parseValue();
	parseSemicolon();

	Statement* post = parseLogic();

	if (tokens->current().type != CALIBURN_T_END_PAREN)
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
/*
Statement* Parser::parseWhile();

Statement* Parser::parseDoWhile();

Statement* Parser::parseSwitch();

Statement* Parser::parseBreak();

Statement* Parser::parseContinue();

Statement* Parser::parsePass();

Statement* Parser::parseReturn();
*/
Statement* Parser::parseValue()
{
	return parseValue(true);
}

Statement* Parser::parseValue(bool doPostfix)
{
	Statement* foundValue = nullptr;

	Token tkn = tokens->current();

	if (tkn.type == CALIBURN_T_KEYWORD)
	{
		tokens->consume();

		if (tkn == "this")
		{
			foundValue = nullptr;//new ThisStatement();
		}
		else if (tkn == "super")
		{
			foundValue = nullptr;//new SuperStatement();
		}
		else
		{
			//TODO complain
			return nullptr;
		}
		//TODO figure out what to do with "new" keyword
		//probably use it in CPU-sided memory allocation
		/*else if (tkn != "new")
		{
			//TODO complain
			return nullptr;
		}

		ParsedType* type = parseTypeName();
		*/
	}
	//field, constructor...
	else if (tkn.type == CALIBURN_T_IDENTIFIER)
	{
		std::string mod = parseNamespace();
		std::string name = tokens->current();
		tokens->consume();

		std::vector<ParsedType*> generics;
		bool isGeneric = parseGenerics(generics);

		if (tokens->current().type == CALIBURN_T_START_PAREN)
		{
			tokens->consume();

			std::vector<Statement*> args;
			parseValueList(args);

			if (tokens->current().type != CALIBURN_T_END_PAREN)
			{
				//TODO complain
				return nullptr;
			}

			tokens->consume();

			//foundValue = new FuncCallStatement();
		}
		else if (isGeneric)
		{
			//TODO complain
			return nullptr;
		}

		//foundValue = new FieldAccessStatement(mod, name);
	}
	//(x)
	else if (tkn.type == CALIBURN_T_START_PAREN)
	{
		tokens->consume();
		foundValue = parseValue();

		if (tokens->current().type != CALIBURN_T_END_PAREN)
		{
			//TODO complain
			return nullptr;
		}

		tokens->consume();

	}
	//[xs...] (array list)
	else if (tkn.type == CALIBURN_T_START_BRACKET)
	{
		tokens->consume();
		std::vector<Statement*> arrayList;

		parseValueList(arrayList);

		if (tokens->current().type != CALIBURN_T_END_BRACKET)
		{
			//TODO complain
			return nullptr;
		}

		tokens->consume();

		//foundValue = new ListValueStatement(arrayList);

	}
	//|x| (abs value)
	else if (tkn.type == CALIBURN_T_LINE)
	{
		tokens->consume();
		foundValue = parseValue();

		if (tokens->current().type != CALIBURN_T_LINE)
		{
			//TODO complain
			return nullptr;
		}

		tokens->consume();
		//foundValue = new AbsOpStatement(foundValue);
	}
	//!x, ~x, -x, etc.
	else if (tkn.type == CALIBURN_T_OPERATOR)
	{
		//!x (bool invert)
		if (tkn == "!")
		{
			tokens->consume();
			//foundValue = new BoolInvOpStatement(parseValue());

		}
		//~x (bitwise invert)
		else if (tkn == "~")
		{
			tokens->consume();
			//foundValue = new BitInvOpStatement(parseValue());

		}
		//-x (int negate)
		else if (tkn == "-")
		{
			tokens->consume();
			//foundValue = new IntNegOpStatement(parseValue());

		}

		//TODO complain
		return nullptr;
	}
	else foundValue = parseLiteral();

	if (!foundValue)
	{
		return nullptr;
	}

	//And now for the postfix operators

	if (!doPostfix)
	{
		return foundValue;
	}

	while (true)
	{
		tkn = tokens->current();

		//x + y
		if (tkn.type == CALIBURN_T_OPERATOR)
		{
			//NOTE not all operators that end in = are a setter (namely == and >=), so this needs a workaround
			if (tkn.str[tkn.str.size() - 1] == '=')
			{
				//nvm we just found a setter, which isn't a valid value
				//so no I don't want stupid things like x = (y = z);
				break;
			}
			
			tokens->consume();

			//fun fact, this is the only reason why the postfix flag is here
			//it's only so that the order of operations is preserved (left to right)
			//I'd love to have it be mathematically accurate, but that logic will be a PAIN to implement
			Statement* rhs = parseValue(false);

			if (!rhs)
			{
				//TODO complain
				delete foundValue;
				return nullptr;
			}

			//foundValue = new ValueOperatorStatement(foundValue, tkn.str, rhs);
		}
		//x[y]
		else if (tkn.type == CALIBURN_T_START_BRACKET)
		{
			tokens->consume();
			Statement* index = parseValue();

			if (!index)
			{
				//TODO complain
				delete foundValue;
				return nullptr;
			}

			if (tokens->current().type != CALIBURN_T_END_BRACKET)
			{
				//TODO complain
				delete foundValue;
				delete index;
				return nullptr;
			}

			//foundValue = new ArrayAccessStatement(value, index);
		}
		//x.y or x.y()
		else if (tkn.type == CALIBURN_T_PERIOD)
		{
			tokens->consume();
			Statement* fieldOrFunc = parseFieldOrFuncValue(false);

			if (!fieldOrFunc)
			{
				//TODO complain
				delete foundValue;
				return nullptr;
			}

			//TODO retarget
			//fieldOrFunc.setTarget(foundValue);
			foundValue = fieldOrFunc;
		}
		else break;
	}
	
	return foundValue;
}

Statement* Parser::parseAnyFieldOrFuncVal()
{
	return parseFieldOrFuncValue(true);
}

Statement* Parser::parseFieldOrFuncValue(bool canHaveNamespace)
{
	ParsedType* type = parseTypeName();

	if (!type)
	{
		return nullptr;
	}

	Token tkn = tokens->current();

	if (!canHaveNamespace && type->mod != "")
	{
		delete type;
		return nullptr;
	}

	tkn = tokens->next();

	if (tkn.type == CALIBURN_T_START_PAREN)
	{
		auto args = new std::vector<Statement*>();
		parseValueList(*args);
		tkn = tokens->current();

		if (tkn.type != CALIBURN_T_END_PAREN)
		{
			//TODO complain
		}

		//return new FuncCall(type, args);
	}

	//return new FieldAccessStatement(mod, name);
}

//Statement* Parser::parseLiteral();
