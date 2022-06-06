
#include "parser.h"

//I'm so sorry
using namespace caliburn;

void Parser::postParseException(CaliburnException* e)
{
	errors.push_back(e);

}

void Parser::parse(std::vector<Token>* tokenList, std::vector<Statement*>* ast)
{
	//don't want to pollute the heap too much
	auto bufferTmp = buffer<Token>(tokenList);
	tokens = &bufferTmp;

	while (tokens->hasNext())
	{
		Token* start = tokens->current();
		Statement* finished = parseDecl();

		if (finished)
		{
			ast->push_back(finished);
		}
		else
		{
			postParseException(new InvalidDeclException(start));
			break;
		}

	}

}

void Parser::parseIdentifierList(std::vector<std::string>& ids)
{
	while (tokens->hasNext())
	{
		Token* tkn = tokens->current();

		if (tkn->type != TokenType::IDENTIFIER)
		{
			break;
		}

		ids.push_back(tkn->str);
		tkn = tokens->next();

		if (tkn->type != TokenType::COMMA)
		{
			break;
		}

		tokens->consume();

	}

}

void Parser::parseValueList(std::vector<ValueStatement*>& xs)
{
	while (true)
	{
		if (tokens->current()->type == TokenType::COMMA)
		{
			continue;
		}

		ValueStatement* val = parseValue();

		if (!val)
		{
			break;
		}

		xs.push_back(val);
		
		tokens->consume();

	}

}

bool Parser::parseArrayList(std::vector<ValueStatement*>& xs)
{
	if (tokens->current()->type != TokenType::START_BRACKET)
	{
		return false;
	}

	tokens->consume();

	parseValueList(xs);

	if (tokens->current()->type != TokenType::END_BRACKET)
	{
		for (auto x : xs)
		{
			delete x;
		}

		xs.clear();

		postParseException(new UnexpectedTokenException(tokens->current(), ']'));

		return false;
	}

	return true;
}

bool Parser::parseGenerics(std::vector<ParsedType*>& generics)
{
	bool valid = false;
	auto oldIndex = tokens->currentIndex();

	if (tokens->current()->str != GENERIC_START)
	{
		return false;
	}

	tokens->consume();

	while (tokens->hasNext())
	{
		auto tkn = tokens->current();

		if (tkn->type == TokenType::COMMA)
		{
			tokens->consume();
			continue;
		}

		if (tkn->str == GENERIC_END)
		{
			tokens->consume();
			valid = true;
			break;
		}

		auto type = parseTypeName();

		if (!type)
		{
			break;
		}

		generics.push_back(type);

	}

	if (generics.size() == 0)
	{
		valid = false;
	}

	if (!valid)
	{
		for (auto type : generics)
		{
			delete type;
		}

		generics.clear();
		tokens->revertTo(oldIndex);

	}

	return valid;
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

Token* Parser::parseNamespace()
{
	if (tokens->current()->type == TokenType::IDENTIFIER &&
		tokens->peek()->type == TokenType::COLON)
	{
		Token* tkn = tokens->current();
		tokens->consume(2);
		return tkn;
	}

	return nullptr;
}

ParsedType* Parser::parseTypeName()
{
	Token* moduleName = parseNamespace();
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		postParseException(new ParseException("Expected identifier; this is NOT a valid identifier.", tkn));
		return nullptr;
	}

	tokens->consume();

	ParsedType* type = new ParsedType(tkn);

	type->mod = moduleName;

	parseGenerics(type->generics);

	return type;
}

bool Parser::parseSemicolon()
{
	Token* tkn = tokens->current();

	if (tkn->type == TokenType::END)
	{
		tokens->consume();
		return true;
	}

	postParseException(new UnexpectedTokenException(tkn, ';'));
	return false;
}

Statement* Parser::parseDecl()
{
	Statement* stmt = parseAny({
		&Parser::parseFunction,
		//&Parser::parseShader,
		//&Parser::parseClass,
		&Parser::parseImport,
		&Parser::parseTypedef,
		//&Parser::parseDescriptor,
		//&Parser::parseStruct,
		&Parser::parseLogic
		});

	if (!stmt)
	{
		postParseException(new ParseException("Invalid start to declaration:", tokens->current()));
	}
	else if (!parseSemicolon())
	{
		postParseException(new ParseException("All declarations must end with a semicolon", tokens->current()));
	}

	return stmt;
}

Statement* Parser::parseImport()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "import")
	{
		return nullptr;
	}

	Token* modName = tokens->next();
	Token* alias = nullptr;
	tkn = tokens->next();

	if (tkn->type == TokenType::KEYWORD &&
		tkn->str == "as")
	{
		alias = tokens->next();

	}

	return new ImportStatement(modName, alias);
}

Statement* Parser::parseTypedef()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "type")
	{
		return nullptr;
	}

	if (tokens->next()->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	std::string name = tokens->current()->str;

	if (tokens->next()->str != "=")
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

//Statement* Parser::parseShader();

//Statement* Parser::parseDescriptor();

//Statement* Parser::parseStruct();

//Statement* Parser::parseClass();

Statement* Parser::parseFunction()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "def")
	{
		return nullptr;
	}

	tkn = tokens->next();

	//parse optional visibility
	StorageModifier mod = parseStorageMod(tkn->str);

	if (mod != StorageModifier::NONE)
	{
		tkn = tokens->next();

	}

	ParsedType* type = parseTypeName();
	std::string name = tokens->current()->str;
	std::vector<ParsedType*> generics;
	std::vector<std::string> gpuThreadData;

	tokens->consume();

	parseGenerics(generics);

	//parse GPU threading data
	if (tkn->str == "[")
	{
		parseIdentifierList(gpuThreadData);

		if (gpuThreadData.size() == 0)
		{
			postParseException(new ParseException("GPU threading data is empty or has invalid values", tkn));
		}

		if (tokens->current()->str != "]")
		{
			postParseException(new UnexpectedTokenException(tokens->current(), ']'));
		}

		tkn = tokens->next();

	}

	if (tkn->str != "(")
	{
		postParseException(new UnexpectedTokenException(tokens->current(), '('));
		return nullptr;
	}

	//TODO parse arg list

	tkn = tokens->next();

	if (tkn->str != ")")
	{
		postParseException(new UnexpectedTokenException(tokens->current(), ')'));
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
	auto stmt = parseAny({ &Parser::parseControl, &Parser::parseScope,
		&Parser::parseAnyVar, /*&Parser::parseSetter*/});

	if (stmt) return stmt;

	stmt = parseFieldOrFuncValue();

	if (!stmt)
	{
		return nullptr;
	}

	//TODO check stmt and complain
	/*
	if (stmt.type == CALIBURN_FIELD_ACCESS)
	{
		//complain
		return nullptr;
	}
	*/
	return stmt;
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

	Token* tkn = tokens->current();

	if (tkn->type == TokenType::KEYWORD)
	{
		if (tkn->str == "let" && implicitAllowed)
		{
			tokens->consume();

		}
		else
		{
			postParseException(new ParseException("Invalid start to variable:", tkn));
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
		postParseException(new ParseException("Invalid name for variable:", tkn));
		return nullptr;
	}

	if (tokens->current()->str == "=")
	{
		tokens->consume();
		defVal = parseValue();
		//type = defVal->type;

	}
	else if (!type)
	{
		postParseException(new ParseException("All implicitly-typed variables must be manually initialized", tkn));
		return nullptr;
	}

	//return new VariableStatement();
}

//Statement* Parser::parseSetter();

Statement* Parser::parseScope()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::START_SCOPE)
	{
		return nullptr;
	}

	tokens->consume();

	ScopeStatement* scope = new ScopeStatement();

	while (tokens->current()->type != TokenType::END_SCOPE)
	{
		scope->innerCode.push_back(parseDecl());
		
	}

	tokens->consume();

	return scope;
}

Statement* Parser::parseControl()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	return parseAny({ &Parser::parseIf, &Parser::parseFor, &Parser::parsePass, /*&Parser::parseWhile,
		&Parser::parseDoWhile, &Parser::parseSwitch */});
}

Statement* Parser::parseIf()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "if")
	{
		return nullptr;
	}

	if (tokens->peek()->type != TokenType::START_PAREN)
	{
		postParseException(new UnexpectedTokenException(tokens->current(), '('));
		return nullptr;
	}

	tokens->consume();

	ValueStatement* condition = parseValue();

	if (tokens->current()->type != TokenType::END_PAREN)
	{
		postParseException(new UnexpectedTokenException(tokens->current(), ')'));
		delete condition;
		return nullptr;
	}

	tokens->consume();

	IfStatement* parsed = new IfStatement();

	parsed->condition = condition;
	parsed->ifBranch = parseLogic();

	if (tokens->current()->str == "else")
	{
		tokens->consume();
		parsed->elseBranch = parseLogic();

	}

	return parsed;
}

Statement* Parser::parseFor()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "for")
	{
		return nullptr;
	}

	if (tokens->next()->type != TokenType::START_PAREN)
	{
		postParseException(new UnexpectedTokenException(tokens->current(), '('));
		return nullptr;
	}

	tokens->consume();

	Statement* pre = parseLogic();
	parseSemicolon();

	ValueStatement* cond = parseValue();
	parseSemicolon();

	Statement* post = parseLogic();

	if (tokens->current()->type != TokenType::END_PAREN)
	{
		postParseException(new UnexpectedTokenException(tokens->current(), ')'));
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

Statement* Parser::parseWhile()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "while")
	{
		return nullptr;
	}

	if (tokens->next()->type != TokenType::START_PAREN)
	{
		postParseException(new UnexpectedTokenException(tokens->current(), '('));
		return nullptr;
	}

	tokens->consume();

	ValueStatement* cond = parseValue();

	if (tokens->current()->type != TokenType::END_PAREN)
	{
		postParseException(new UnexpectedTokenException(tokens->current(), ')'));
		delete cond;
		return nullptr;
	}

	tokens->consume();

	Statement* loop = parseLogic();

	WhileStatement* stmt = new WhileStatement();
	
	stmt->cond = cond;
	stmt->loop = loop;

	return stmt;
}

Statement* Parser::parseDoWhile()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "do")
	{
		return nullptr;
	}

	tokens->consume();

	Statement* body = parseLogic();

	tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "while")
	{
		delete body;
		return nullptr;
	}

	tkn = tokens->next();

	if (tkn->type != TokenType::START_PAREN)
	{
		//TODO complain
		delete body;
		return nullptr;
	}

	tokens->consume();

	Statement* cond = parseValue();

	tkn = tokens->current();

	if (tkn->type != TokenType::END_PAREN)
	{
		//TODO complain
		delete body;
		delete cond;
		return nullptr;
	}

	tokens->consume();

	auto ret = new DoWhileStatement();

	ret->loop = body;
	ret->cond = (ValueStatement*)cond;

	return ret;
}

Statement* Parser::parseBreak()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "break")
	{
		return nullptr;
	}

	tokens->consume();

	return new BreakStatement();
}

Statement* Parser::parseContinue()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "continue")
	{
		return nullptr;
	}

	tokens->consume();
	
	return new ContinueStatement();
}

Statement* Parser::parseSwitch()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "switch")
	{
		return nullptr;
	}

	tkn = tokens->next();

	if (tkn->type != TokenType::START_PAREN)
	{
		//TODO complain
		return nullptr;
	}

	SwitchStatement* stmnt = new SwitchStatement();

	tokens->consume();
	ValueStatement* swValue = parseValue();

	if (!swValue)
	{
		//TODO complain
		delete stmnt;
		return nullptr;
	}

	stmnt->condition = swValue;

	tkn = tokens->current();

	if (tkn->type != TokenType::END_PAREN)
	{
		//TODO complain
		delete stmnt;
		return nullptr;
	}

	tkn = tokens->next();
	
	if (tkn->type != TokenType::START_SCOPE)
	{
		//TODO complain
		delete stmnt;
		return nullptr;
	}

	while (tokens->hasNext())
	{
		Statement* swCase = parseCase();

		if (!swCase)
		{
			break;
		}

		stmnt->cases.push_back(swCase);

	}

	tkn = tokens->current();

	if (tkn->type != TokenType::END_SCOPE)
	{
		//TODO complain
		delete stmnt;
		return nullptr;
	}

	return stmnt;
}

Statement* Parser::parseCase()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "case")
	{
		return nullptr;
	}

	CaseStatement* stmnt = new CaseStatement();
	ValueStatement* caseVal = parseValue();

	if (!caseVal)
	{
		//TODO complain
		delete stmnt;
		return nullptr;
	}

	stmnt->condition = caseVal;

	tkn = tokens->current();

	if (tkn->type != TokenType::COLON)
	{
		//TODO complain
		delete stmnt;
		return nullptr;
	}

	tokens->consume();

	while (tokens->hasNext())
	{
		Statement* logic = parseLogic();
		
		if (!logic)
		{
			break;
		}

		stmnt->logic.push_back(logic);

		if (!parseSemicolon())
		{
			//TODO complain
			delete stmnt;
			return nullptr;
		}

	}

	return stmnt;
}

Statement* Parser::parsePass()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "pass")
	{
		return nullptr;
	}

	return new PassStatement();
}

Statement* Parser::parseReturn()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "return")
	{
		return nullptr;
	}

	ReturnStatement* ret = new ReturnStatement();

	tkn = tokens->next();

	//technically cheating but w/e
	if (tkn->type != TokenType::END)
	{
		ret->val = parseValue();
	}

	return ret;
}

Statement* Parser::parseStmtInParentheses(ParseMethod pm)
{
	Token* tkn = tokens->current();

	if (tkn->str != "(")
	{
		return nullptr;
	}

	tokens->consume();
	auto found = (this->*pm)();

	if (tokens->current()->type != TokenType::END_PAREN)
	{
		postParseException(new UnexpectedTokenException(tokens->current(), ')'));
		delete found;
		return nullptr;
	}

	tokens->consume();

	return found;
}

ValueStatement* Parser::parseAnyValue()
{
	return parseValue(true);
}

//TODO parse using Shunting-yard algorithm (or at least check for compliance)
//https://en.wikipedia.org/wiki/Shunting-yard_algorithm
ValueStatement* Parser::parseValue(bool doPostfix)
{
	ValueStatement* foundValue = nullptr;
	std::vector<Statement*> dispatchParams;

	Token* tkn = tokens->current();

	if (tkn->type == TokenType::KEYWORD)
	{
		tokens->consume();

		if (tkn->str == "make")
		{
			/*
			auto type = parseTypeName();
			tkn = tokens->current();

			if (tkn->type != TokenType::START_PAREN)
			{
				//TODO complain
			}

			tokens->consume();

			std::vector<ValueStatement*> cargs;

			parseValueList(cargs);

			tkn = tokens->current();
			if (tkn->type != TokenType::END_PAREN)
			{
				//TODO complain
			}

			tokens->consume();
			*/
			foundValue = nullptr;//new StackAllocStatement(parseType());
		}
		if (tkn->str == "this")
		{
			foundValue = nullptr;//new ThisStatement();
		}
		else if (tkn->str == "super")
		{
			foundValue = nullptr;//new SuperStatement();
		}
		else
		{
			//postParseException(new ParseException("Invalid value keyword:", tkn));
			return nullptr;
		}
		//TODO figure out what to do with "new" keyword
		//probably use it in CPU-sided memory allocation
		/*else if (tkn->str == "new")
		{
			return new HeapAllocStatement(parseType());
		}
		//TODO complain
		*/
	}
	//field, constructor...
	else if (tkn->type == TokenType::IDENTIFIER)
	{
		std::vector<Token*> modChain;

		while (tokens->current()->type == TokenType::IDENTIFIER && tokens->peek()->type == TokenType::COLON)
		{
			modChain.push_back(tokens->current());
			tokens->consume(2);
			
		}

		foundValue = (ValueStatement*)parseFieldOrFuncValue();

		for (auto mod = modChain.rbegin(); mod != modChain.rend(); ++mod)
		{
			foundValue = new ModuleReadStatement(*mod, foundValue);

		}

	}
	//(x)
	else if (tkn->type == TokenType::START_PAREN)
	{
		foundValue = parseValueInParentheses();

	}
	//[xs...] (array list)
	else if (tkn->type == TokenType::START_BRACKET)
	{
		tokens->consume();
		std::vector<ValueStatement*> arrayList;

		parseValueList(arrayList);

		if (tokens->current()->type != TokenType::END_BRACKET)
		{
			return nullptr;
		}

		tokens->consume();

		//foundValue = new ListValueStatement(arrayList);

	}
	//|x| (abs value)
	else if (tkn->str == "|")
	{
		tokens->consume();
		foundValue = parseValue();

		if (tokens->current()->str != "|")
		{
			postParseException(new ParseException("Absolute values must be surrounded by pipes ('|')", tokens->current()));
			return nullptr;
		}

		tokens->consume();
		//foundValue = new AbsOpStatement(foundValue);
	}
	//!x, ~x, -x, etc.
	else if (tkn->type == TokenType::MATH_OPERATOR)
	{
		//!x (bool invert)
		if (tkn->str == "!")
		{
			tokens->consume();
			//foundValue = new BoolInvOpStatement(parseValue());

		}
		//~x (bitwise invert)
		else if (tkn->str == "~")
		{
			tokens->consume();
			//foundValue = new BitInvOpStatement(parseValue());

		}
		//-x (int negate)
		else if (tkn->str == "-")
		{
			tokens->consume();
			//foundValue = new IntNegOpStatement(parseValue());

		}

		postParseException(new ParseException("Invalid prefix operator; try !, ~, -, or |value|", tokens->current()));
		return nullptr;
	}
	//else foundValue = parseLiteral();

	if (!foundValue)
	{
		return nullptr;
	}

	//*technically* these are postfix, but really postfix is just for math operators.

	while (true)
	{
		tkn = tokens->current();

		if (tkn->type == TokenType::START_BRACKET)
		{
			std::vector<ValueStatement*> indices;

			if (!parseArrayList(indices) || indices.size() == 0)
			{
				postParseException(new ParseException("Invalid array-style accessor value starts here:", tkn));
				delete foundValue;
				return nullptr;
			}

			//foundValue = new ArrayAccessStatement(value, index);
		}
		//x.y or x.y()
		else if (tkn->type == TokenType::PERIOD)
		{
			tkn = tokens->next();
			Statement* fieldOrFunc = parseFieldOrFuncValue();

			if (!fieldOrFunc)
			{
				postParseException(new ParseException("Invalid attempt to access a member:", tkn));
				delete foundValue;
				return nullptr;
			}

			//TODO retarget
			//fieldOrFunc.setTarget(foundValue);
			foundValue = (ValueStatement*)fieldOrFunc;
		}
		else break;
	}

	if (!doPostfix)
	{
		return foundValue;
	}

	while (true)
	{
		tkn = tokens->current();

		//x + y
		if (tkn->type == TokenType::MATH_OPERATOR || tkn->type == TokenType::LOGIC_OPERATOR)
		{
			if (tkn->type == TokenType::MATH_OPERATOR && tkn->str[tkn->str.size() - 1] == '=')
			{
				//nvm we just found a setter, which isn't a valid value
				//so no I don't want stupid things like x = (y = z);
				break;
			}

			tkn = tokens->next();

			//fun fact, this is the only reason why the postfix flag is here
			//it's only so that the order of operations is preserved (left to right)
			//I'd love to have it be mathematically accurate, but that logic will be a PAIN to implement
			Statement* rhs = parseValue(false);

			if (!rhs)
			{
				postParseException(new ParseException("Invalid value for RHS of equation starts here", tkn));
				delete foundValue;
				return nullptr;
			}

			//foundValue = new ValueOperatorStatement(foundValue, tkn.str, rhs);
		}
		else break;
	}
	
	return foundValue;
}

ValueStatement* Parser::parseValueInParentheses()
{
	return (ValueStatement*)parseStmtInParentheses((ParseMethod)&Parser::parseAnyValue);
}

Statement* Parser::parseFieldOrFuncValue()
{
	Token* tkn = tokens->current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	Token* name = tkn;

	tkn = tokens->next();
	std::vector<ValueStatement*> accessors;

	if (tkn->type == TokenType::START_BRACKET)
	{
		//either an array accessor or dispatch

		if (!parseArrayList(accessors) || accessors.size() == 0)
		{
			postParseException(new ParseException("Invalid array accessor (or dispatch) value(s) start at:", tkn));
			return nullptr;
		}

	}

	if (tokens->current()->type == TokenType::START_PAREN)
	{
		std::vector<ValueStatement*> args;
		parseValueList(args);

		tkn = tokens->current();
		
		if (tkn->type != TokenType::END_PAREN)
		{
			postParseException(new UnexpectedTokenException(tkn, ')'));
		}

		if (accessors.size() > 0)
		{
			//return new GPUDispatch();
		}

		//return new FuncCall(type, args);
	}
	/*else if (type->generics.size() > 0)
	{
		//someone's trying to use a type name as a field
		//extremely weird but OK
		postParseException(new ParseException("Valid(ish) type with generic found, but being used as a field and not a constructor", tkn));
		delete type;
		return nullptr;
	}*/

	//auto found = new MemberReadStatement(type->mod, type->name);

	/*
	if (accessors.size() > 0)
	{
		found = new ArrayAccess(found, accessors);
	}
	*/

	//return found;
}

//Statement* Parser::parseLiteral();
