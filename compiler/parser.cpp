
#include "parser.h"

#include "ctrlstmt.h"
#include "modstmts.h"
#include "structstmt.h"
#include "typestmt.h"
#include "valstmt.h"

using namespace caliburn;

void Parser::postParseException(CaliburnException* e)
{
	errors.push_back(e);

}

void Parser::parse(ptr<std::vector<Token>> tokenList, ptr<std::vector<ptr<Statement>>> ast)
{
	//don't want to pollute the heap
	auto bufferTmp = buffer<Token>(tokenList);
	tokens = &bufferTmp;

	while (tokens->hasNext())
	{
		auto start = tokens->current();
		ptr<Statement> finished = parseDecl();

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

template<typename T>
ptr<T> Parser::parseAny(std::initializer_list<ParseMethod<T>> fns)
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
		T* parsed = (this->*fn)();

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

template<typename T>
ptr<T> Parser::parseBetween(std::string start, ParseMethod<T> fn, std::string end)
{
	if (tokens->current()->str != start)
	{
		//TODO complain
		return nullptr;
	}

	tokens->consume();

	T* ret = (this->*fn)();

	if (tokens->current()->str != end)
	{
		//TODO complain
		return ret;
	}

	tokens->consume();

	return ret;
}

void Parser::parseAnyBetween(std::string start, std::function<void()> fn, std::string end)
{
	if (tokens->current()->str != start)
	{
		//TODO complain
		return;
	}

	tokens->consume();

	fn();

	if (tokens->current()->str != end)
	{
		//TODO complain
		return;
	}

	tokens->consume();

}

void Parser::parseIdentifierList(ref<std::vector<ptr<Token>>> ids)
{
	while (tokens->hasNext())
	{
		auto tkn = tokens->current();

		if (tkn->type != TokenType::IDENTIFIER)
		{
			break;
		}

		ids.push_back(tkn);
		tkn = tokens->next();

		if (tkn->type != TokenType::COMMA)
		{
			break;
		}

		tokens->consume();

	}

}

bool Parser::parseGenerics(ref<std::vector<ptr<ParsedType>>> generics)
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

bool Parser::parseValueList(ref<std::vector<ptr<Value>>> values, bool commaOptional)
{
	while (tokens->hasNext())
	{
		values.push_back(parseAnyValue());

		if (tokens->current()->str == ",")
		{
			tokens->consume();
			continue;
		}
		else if (!commaOptional)
		{
			break;
		}

	}

	return true;
}

bool Parser::parseSemicolon()
{
	auto tkn = tokens->current();

	if (tkn->type == TokenType::END)
	{
		tokens->consume();
		return true;
	}

	//postParseException(new UnexpectedTokenException(tkn, ';'));
	return false;
}

bool Parser::parseScopeEnd(ptr<ScopeStatement> stmt)
{
	auto tkn = tokens->current();
	auto tknIndex = tokens->currentIndex();

	if (tkn->type != TokenType::KEYWORD)
	{
		return false;
	}

	bool valid = true;

	tokens->consume();

	if (tkn->str == "return")
	{
		stmt->retMode = ReturnMode::RETURN;
		tokens->consume();
		stmt->retValue = parseAnyValue();

	}
	else if (tkn->str == "break")
	{
		stmt->retMode = ReturnMode::BREAK;

	}
	else if (tkn->str == "continue")
	{
		stmt->retMode = ReturnMode::CONTINUE;

	}
	else if (tkn->str == "pass")
	{
		stmt->retMode = ReturnMode::PASS;

	}
	else if (tkn->str == "unreachable")
	{
		stmt->retMode = ReturnMode::UNREACHABLE;

	}
	else
	{
		valid = false;
	}

	if (valid)
	{
		tokens->consume();
		if (!parseSemicolon())
		{
			//TODO complain
			return false;
		}

		return true;
	}

	tokens->revertTo(tknIndex);

	return false;
}

StorageModifiers Parser::parseStorageMods()
{
	StorageModifiers mods = {};

	while (tokens->hasNext())
	{
		auto tkn = tokens->current();

		if (tkn->str == "public")
			mods.PUBLIC = 1;
		else if (tkn->str == "private")
			mods.PRIVATE = 1;
		else if (tkn->str == "protected")
			mods.PROTECTED = 1;
		else if (tkn->str == "shared")
			mods.SHARED = 1;
		else if (tkn->str == "static")
			mods.STATIC = 1;
		else
			break;

		tokens->consume();

	}

	return mods;
}

ptr<ScopeStatement> Parser::parseScope(std::initializer_list<ParseMethod<Statement>> pms)
{
	auto mods = parseStorageMods();

	auto tkn = tokens->current();

	if (tkn->type != TokenType::START_SCOPE)
	{
		return nullptr;
	}

	auto scope = new ScopeStatement();

	tokens->consume();

	while (tokens->hasNext())
	{
		tkn = tokens->current();

		if (tkn->type == TokenType::END_SCOPE)
		{
			tokens->consume();
			break;
		}

		ptr<Statement> stmt = nullptr;

		for (auto pm : pms)
		{
			stmt = (this->*pm)();

			if (stmt != nullptr)
			{
				break;
			}

		}

		if (stmt == nullptr)
		{
			//TODO complain
			break;
		}

		if (!parseSemicolon())
		{
			//TODO complain but continue parsing
		}

		scope->stmts.push_back(stmt);

	}

	return scope;
}

ptr<ParsedType> Parser::parseTypeName()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		postParseException(new ParseException("Expected identifier; this is NOT a valid identifier.", tkn));
		return nullptr;
	}

	tokens->consume();

	ptr<ParsedType> type = new ParsedType(tkn);

	parseGenerics(type->generics);

	tkn = tokens->current();

	while (tokens->hasNext() && tkn->type == TokenType::START_BRACKET)
	{
		tkn = tokens->next();

		ptr<Value> len = nullptr;

		if (tkn->type != TokenType::END_BRACKET)
		{
			len = parseAnyValue();

			tkn = tokens->current();

		}

		if (tkn->type != TokenType::END_BRACKET)
		{
			//TODO complain
		}

		type->arrayDims.push_back(len);
		type->lastToken = tkn;

		tkn = tokens->next();

	}

	return type;
}

ptr<Statement> Parser::parseDecl()
{
	auto mods = parseStorageMods();

	auto stmt = parseAny({
		&Parser::parseImport,
		&Parser::parseModuleDef,
		&Parser::parseTypedef,
		&Parser::parseFunction,
		/*&Parser::parseShader,
		&Parser::parseClass,*/
		&Parser::parseStruct,
		&Parser::parseLogic
		});

	if (stmt == nullptr)
	{
		postParseException(new ParseException("Invalid start to declaration:", tokens->current()));
		return stmt;
	}

	stmt->mods = mods;

	if (!parseSemicolon())
	{
		postParseException(new ParseException("All declarations must end with a semicolon", tokens->current()));
	}

	return stmt;
}

ptr<Statement> Parser::parseImport()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "import")
	{
		return nullptr;
	}

	auto modName = tokens->next();
	
	ptr<Token> alias = nullptr;

	tkn = tokens->next();

	if (tkn->type == TokenType::KEYWORD &&
		tkn->str == "as")
	{
		if (tokens->peek()->type != TokenType::IDENTIFIER)
		{
			//TODO complain
		}

		alias = tokens->next();

		tokens->consume();

	}

	auto ret = new ImportStatement(tkn);

	ret->name = modName;
	ret->alias = alias;

	return ret;
}

ptr<Statement> Parser::parseModuleDef()
{
	auto start = tokens->current();

	if (start->type != TokenType::KEYWORD || start->str != "module")
	{
		return nullptr;
	}

	auto name = tokens->next();

	return new ModuleStatement(start, name);
}

ptr<Statement> Parser::parseTypedef()
{
	auto start = tokens->current();

	if (start->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (start->str == "strong")
	{
		tokens->consume();
		
	}

	if (tokens->current()->str != "type")
	{
		return nullptr;
	}

	if (tokens->next()->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	auto name = tokens->current();

	if (tokens->next()->str != "=")
	{
		return nullptr;
	}

	tokens->consume();

	ptr<ParsedType> aliasedType = parseTypeName();

	auto stmt = new TypedefStatement(start, name, aliasedType);
	
	return stmt;
}

//ptr<Statement> Parser::parseShader();

//ptr<Statement> Parser::parseDescriptor();

//TODO clean up architecture, make expandable
ptr<Statement> Parser::parseStruct()
{
	bool isConst = false;
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (tkn->str == "record")
	{
		isConst = true;
	}
	else if (tkn->str != "struct")
	{
		return nullptr;
	}

	auto name = tokens->next();

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
	}

	auto ret = new StructStatement(name, isConst ? StatementType::RECORD : StatementType::STRUCT);

	ret->first = tkn;

	tkn = tokens->next();

	if (tkn->str != "{")
	{
		//TODO complain
	}

	while (tokens->hasNext())
	{
		tkn = tokens->next();

		if (tkn->str == "}")
		{
			ret->last = tkn;
			tokens->consume();
			break;
		}

		auto mem = parseMemberVar();

		if (mem != nullptr)
		{
			mem->isConst = isConst;
			ret->members.push_back(mem);

		}

		if (!parseSemicolon())
		{
			//TODO complain
		}

		//TODO parse constructors(?)
		//does a struct even have a constructor?

	}

	return ret;
}

//ptr<Statement> Parser::parseClass();

ptr<Statement> Parser::parseFunction()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "def")
	{
		return nullptr;
	}

	tkn = tokens->next();

	ptr<ParsedType> type = parseTypeName();
	std::string name = tokens->current()->str;
	std::vector<ptr<ParsedType>> generics;
	std::vector<ptr<Token>> gpuThreadData;

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

	ptr<Statement> body = parseLogic();

	if (!body)
	{
		return nullptr;
	}
	/*
	Functionptr<Statement> func = new FunctionStatement();

	func->type = type;
	func->name = name;
	func->funcBody = body;
	//TODO finish

	return func;*/
	return nullptr;
}

ptr<Statement> Parser::parseMethod()
{
	return parseAny({
		&Parser::parseFunction,
		&Parser::parseConstructor,
		&Parser::parseDestructor
		});
}

ptr<Statement> Parser::parseConstructor()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "construct")
	{
		return nullptr;
	}

	return nullptr;
}

ptr<Statement> Parser::parseDestructor()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "destruct")
	{
		return nullptr;
	}

	return nullptr;
}

//ptr<Statement> parseOp();

ptr<Statement> Parser::parseLogic()
{
	return parseAny({ &Parser::parseControl, &Parser::parseValueStmt });
}

ptr<Statement> Parser::parseControl()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	return parseAny({ &Parser::parseIf, &Parser::parseFor, &Parser::parseWhile,
		&Parser::parseDoWhile, /* parseSwitch */ });
}

ptr<Statement> Parser::parseIf()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "if")
	{
		return nullptr;
	}

	tokens->consume();

	auto parsed = new IfStatement();

	parsed->condition = parseBetween("(", &Parser::parseAnyValue, ")");
	parsed->innerIf = parseScope({&Parser::parseDecl});

	if (tokens->current()->str == "else")
	{
		tokens->consume();

		auto innerElseStmt = parseLogic();

		if (innerElseStmt != nullptr)
		{
			parsed->innerElse = new ScopeStatement();

			parsed->innerElse->stmts.push_back(innerElseStmt);

		}
		else
		{
			parsed->innerElse = parseScope({&Parser::parseDecl});

		}

	}

	return parsed;
}

ptr<Statement> Parser::parseFor()
{
	return nullptr;
}

ptr<Statement> Parser::parseWhile()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "while")
	{
		return nullptr;
	}

	auto cond = parseBetween("(", &Parser::parseAnyValue, ")");

	auto stmt = new WhileStatement();
	
	stmt->first = tkn;
	stmt->condition = cond;
	stmt->loop = parseScope({ &Parser::parseLogic });

	return stmt;
}

ptr<Statement> Parser::parseDoWhile()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "do")
	{
		return nullptr;
	}

	tokens->consume();

	auto body = parseScope({&Parser::parseLogic});

	if (tokens->current()->type != TokenType::KEYWORD || tokens->current()->str != "while")
	{
		//TODO complain
		delete body;
		return nullptr;
	}

	ptr<Value> cond = parseBetween("(", &Parser::parseAnyValue, ")");

	if (cond == nullptr)
	{
		//TODO complain
		delete body;
		return nullptr;
	}

	auto ret = new WhileStatement();

	ret->doWhile = true;
	ret->loop = body;
	ret->condition = cond;

	return ret;
}

ptr<Statement> Parser::parseValueStmt()
{
	auto val = parseAny({ &Parser::parseAnyFnCall, &Parser::parseAnyExpr });

	if (val == nullptr)
	{
		return nullptr;
	}

	return new ValueStatement(val);
}

ptr<Value> Parser::parseAnyValue()
{
	return parseAny({ &Parser::parseAnyExpr, &Parser::parseLiteral, &Parser::parseAnyFnCall });
}

ptr<Value> Parser::parseNonExpr()
{
	auto tkn = tokens->current();

	ptr<Value> v = parseAny({ &Parser::parseLiteral, &Parser::parseAnyFnCall });

	if (v == nullptr)
	{
		if (tkn->type == TokenType::START_PAREN)
		{
			v = parseBetween("(", &Parser::parseNonExpr, ")");
		}
		else if (tkn->type == TokenType::KEYWORD)
		{
			tokens->consume();

			if (tkn->str == "this")
			{
				v = new VarReadValue(tkn);
			}
			else if (tkn->str == "sign")
			{
				v = new UnaryValue(tkn, Operator::SIGN, parseNonExpr());
			}
			else if (tkn->str == "unsign")
			{
				v = new UnaryValue(tkn, Operator::UNSIGN, parseNonExpr());
			}
			else
			{
				tokens->rewind();
				return nullptr;
			}

		}
		else if (tkn->type == TokenType::OPERATOR)
		{
			Operator uOp = Operator::UNKNOWN;
			bool isUnary = true;

			switch (tkn->str[0])
			{
			case '|': uOp = Operator::ABS; break;
			case '-': uOp = Operator::NEG; break;
			case '~': uOp = Operator::BIT_NEG; break;
			case '!': uOp = Operator::BOOL_NOT; break;
			default: {
				isUnary = false;
				break;
			};
			}

			if (!isUnary)
			{
				return nullptr;
			}

			tokens->consume();

			auto unary = new UnaryValue();

			unary->op = uOp;
			unary->val = parseNonExpr();

			if (uOp == Operator::ABS)
			{
				if (tokens->current()->str != "|")
				{
					//TODO complain
					delete unary;
					return nullptr;
				}

				tokens->consume();

			}

			v = unary;

		}
		else
		{
			return nullptr;
		}

	}

	if (v == nullptr)
	{
		return nullptr;
	}
	
	while (tokens->hasNext())
	{
		tkn = tokens->current();

		if (tkn->type == TokenType::START_BRACKET)
		{
			auto i = parseBetween("[", &Parser::parseAnyValue, "]");//enables for expressions inside of array access
			
			auto subA = new SubArrayValue();

			subA->array = v;
			subA->index = i;
			subA->last = tokens->prev();

			v = subA;

		}
		else if (tkn->type == TokenType::PERIOD && tokens->peek()->type == TokenType::IDENTIFIER)
		{
			tkn = tokens->next();
			auto pk = tokens->peek();

			if (pk->type == TokenType::START_PAREN || pk->str == GENERIC_START)
			{
				v = parseFnCall(v);

			}
			else
			{
				auto memRead = new MemberReadValue();

				memRead->target = v;
				memRead->memberName = tkn;

				v = memRead;

				tokens->consume();

			}

		}
		else
		{
			break;
		}
		
	}
	
	return v;
}

ptr<Value> Parser::parseLiteral()
{
	auto tkn = tokens->current();

	//"this" won't be counted here; it's only a keyword for syntax highlighting purposes
	//Instead, a MethodStatement will add a "this" argument to its own argument list
	if (tkn->type == TokenType::KEYWORD)
	{
		if (tkn->str == "null")
		{
			return new NullValue(tkn);
		}

	}

	switch (tkn->type)
	{
	case TokenType::LITERAL_INT: return new IntLiteralValue(*tkn);
	case TokenType::LITERAL_FLOAT: return new FloatLiteralValue(*tkn);
	case TokenType::LITERAL_BOOL: return new BoolLitValue(*tkn);
	case TokenType::LITERAL_STR: return new StringLitValue(*tkn);
	}

	if (tkn->str == "[")
	{
		auto arrLit = new ArrayLitValue();

		arrLit->start = tkn;

		while (tokens->hasNext())
		{
			tkn = tokens->next();

			if (tkn->str == ",")
			{
				continue;
			}

			if (tkn->str == "]")
			{
				arrLit->end = tkn;
				tokens->consume();
				break;
			}

			auto val = parseAnyValue();

			arrLit->values.push_back(val);

		}

		return arrLit;
	}

	return nullptr;
}

ptr<Value> Parser::parseAnyExpr()
{
	return parseExpr(OP_PRECEDENCE_MAX);
}

ptr<Value> Parser::parseExpr(uint32_t precedence)
{
	auto lhs = parseNonExpr();

	while (tokens->hasNext())
	{
		auto tkn = tokens->current();

		while (tkn->type == TokenType::KEYWORD)
		{
			if (tkn->str == "is")
			{
				tokens->consume();

				auto isa = new IsAValue();

				isa->val = lhs;
				isa->chkPType = parseTypeName();

				lhs = isa;

			}
			else if (tkn->str == "as")
			{
				tokens->consume();

				auto cast = new CastValue();

				cast->lhs = lhs;
				cast->resultPType = parseTypeName();

				lhs = cast;

			}
			else
			{
				break;
			}

		}

		if (tkn->type != TokenType::OPERATOR)
		{
			break;
		}

		auto op = infixOps.find(tkn->str);

		if (op == infixOps.end())
		{
			return lhs;
		}

		auto opWeight = opPrecedence.at(op->second);

		if (opWeight > precedence)
		{
			break;
		}

		tkn = tokens->next();

		//Yes, we use *this* code to parse setters.
		if (tkn->str == "=")
		{
			tokens->consume();

			auto set = new SetterValue();

			auto expr = new ExpressionValue();

			expr->lValue = lhs;
			expr->op = op->second;
			expr->rValue = parseAnyExpr();

			set->lhs = lhs;
			set->rhs = expr;

			return set;
		}
		
		auto expr = new ExpressionValue();

		expr->lValue = lhs;
		expr->op = op->second;
		expr->rValue = parseExpr(opWeight);

		lhs = expr;

	}

	return lhs;
}

ptr<Value> Parser::parseAnyFnCall()
{
	return parseFnCall(nullptr);
}

ptr<Value> Parser::parseFnCall(ptr<Value> start)
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	auto name = tkn;
	std::vector<ptr<ParsedType>> pGenerics;

	tkn = tokens->next();
	
	if (tkn->str == GENERIC_START)
	{
		if (!parseGenerics(pGenerics))
		{
			return nullptr;
		}

	}

	tkn = tokens->current();

	if (tkn->type != TokenType::START_PAREN)
	{
		return nullptr;
	}

	tokens->consume();

	auto ret = new FnCallValue();

	ret->name = name;
	ret->target = start;
	ret->pGenerics = pGenerics;

	if (!parseValueList(ret->args, false))
	{
		//TODO complain
	}

	ret->end = tokens->current();

	return ret;
}

ptr<Variable> Parser::parseLocalVar()
{
	ptr<ParsedType> type = nullptr;
	ptr<Token> name = nullptr;
	ptr<Value> initVal = nullptr;
	bool constant = false;

	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (tkn->str == "var")
	{
		tkn = tokens->current();

	}
	else if (tkn->str == "const")
	{
		tkn = tokens->current();
		constant = true;
	}
	else
	{
		return nullptr;
	}

	if (tkn->str == ":")
	{
		tkn = tokens->next();
		type = parseTypeName();

		if (type == nullptr)
		{
			//TODO complain
		}

	}
	
	name = tokens->next();

	if (name->type != TokenType::IDENTIFIER)
	{
		postParseException(new ParseException("Invalid name for a variable:", name));
		return nullptr;
	}

	tkn = tokens->next();

	if (tkn->str == ":")
	{
		postParseException(new ParseException("That's not how type hints work in Caliburn. Type hints go before the variable name, not after. So var: int x is valid. var x: int isn't.", tkn));
		return nullptr;
	}

	if (tkn->str == "=")
	{
		tokens->consume();
		initVal = parseAnyValue();
		//type = defVal->type;

	}
	else if (!type)
	{
		postParseException(new ParseException("All implicitly-typed variables must be manually initialized", tkn));
		return nullptr;
	}

	if (!parseSemicolon())
	{
		//TODO complain
	}

	return new LocalVariable(name, type, initVal, constant);
}

ptr<Variable> Parser::parseMemberVar()
{
	bool isConst = false;
	ptr<Token> start = tokens->current();

	if (start->type == TokenType::KEYWORD && start->str == "const")
	{
		isConst = true;
		tokens->consume();
	}

	auto type = parseTypeName();

	if (type == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	auto name = tokens->current();
	ptr<Value> initVal = nullptr;

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		delete type;
		return nullptr;
	}

	tokens->consume();

	if (tokens->current()->str == "=")
	{
		tokens->consume();
		initVal = parseAnyValue();

	}

	return new MemberVariable(name, type, initVal, isConst);
}
