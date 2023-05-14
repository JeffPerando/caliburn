
#include "parser.h"

#include "ctrlstmt.h"
#include "modstmts.h"
#include "structstmt.h"
#include "typestmt.h"
#include "valstmt.h"

using namespace caliburn;

void Parser::postParseException(uptr<CaliburnException> e)
{
	errors.push_back(std::move(e));

}

void Parser::parse(ref<std::vector<sptr<Token>>> tokenList, ref<std::vector<uptr<Statement>>> ast)
{
	//don't want to pollute the heap
	auto bufferTmp = buffer<sptr<Token>>(&tokenList);
	tokens = &bufferTmp;

	while (tokens->hasNext())
	{
		auto start = tokens->current();
		auto finished = parseDecl();

		if (finished)
		{
			ast.push_back(std::move(finished));
		}
		else
		{
			postParseException(std::make_unique<InvalidDeclException>(start));
			break;
		}

	}

	tokens = nullptr;

}

template<typename T>
uptr<T> Parser::parseAny(std::initializer_list<ParseMethod<T>> fns)
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
		auto parsed = (this->*fn)();

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
uptr<T> Parser::parseBetween(std::string start, ParseMethod<T> fn, std::string end)
{
	if (tokens->current()->str != start)
	{
		//TODO complain
		return nullptr;
	}

	tokens->consume();

	auto ret = (this->*fn)();

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

void Parser::parseIdentifierList(ref<std::vector<sptr<Token>>> ids)
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

bool Parser::parseGenericSig(ref<GenericSignature> sig)
{
	auto oldIndex = tokens->currentIndex();
	auto tkn = tokens->current();
	bool valid = false;
	
	if (tkn->str != GENERIC_START)
	{
		return false;
	}

	sig.first = tkn;

	bool parsingDefaults = false;

	while (tokens->hasNext())
	{
		tkn = tokens->next();

		if (tkn->type != TokenType::KEYWORD)
		{
			valid = false;
			break;
		}

		if (tkn->str != "type" && tkn->str != "const")
		{
			valid = false;
			break;
		}

		auto genName = GenericName();

		genName.type = tkn;
		genName.name = tokens->next();

		if (genName.name->type != TokenType::IDENTIFIER)
		{
			valid = false;
			break;
		}

		tkn = tokens->next();

		if (tkn->str == "=")
		{
			parsingDefaults = true;

			tokens->consume();

			auto v = parseLiteral();

			if (v != nullptr)
			{
				genName.defaultResult = std::move(v);

			}
			else
			{
				auto t = parseTypeName();

				if (t == nullptr)
				{
					//TODO complain
					break;
				}

				genName.defaultResult = std::move(t);

			}

			tkn = tokens->current();//parse methods should leave us at the first invalid token for that parse

		}
		else
		{
			++sig.minArgs;

			if (parsingDefaults)
			{
				//TODO complain
				//Generics must always have defaults at the end
				//I may reconsider this rule if explicit-named generics become a thing. But that logic is too hard right now
			}

		}

		/*
		NOTE: Because this name contains unique_ptrs, it may be prudent to ensure that the needed copies are handled correctly
		*/
		sig.names.push_back(std::move(genName));

		if (tkn->str != ",")
		{
			break;
		}

		tokens->consume();

	}

	tkn = tokens->current();

	if (tkn->str != GENERIC_END)
	{
		tokens->revertTo(oldIndex);
		valid = false;
	}

	sig.last = tkn;
	sig.isValid = valid;

	return valid;
}

bool Parser::parseGenericArgs(ref<GenericArguments> args)
{
	auto oldIndex = tokens->currentIndex();
	auto tkn = tokens->current();
	bool valid = false;

	if (tkn->str != GENERIC_START)
	{
		return false;
	}

	args.first = tkn;

	while (tokens->hasNext())
	{
		tkn = tokens->next();

		GenericResult result;

		auto v = parseLiteral();

		if (v != nullptr)
		{
			result = std::move(v);

		}
		else
		{
			auto t = parseTypeName();

			if (t == nullptr)
			{
				//TODO complain
				break;
			}

			result = std::move(t);

		}

		tkn = tokens->current();//parse methods should leave us at the first invalid token for that parse

		args.args.push_back(std::move(result));

		if (tkn->str != ",")
		{
			break;
		}

		tokens->consume();

	}

	tkn = tokens->current();

	if (tkn->str != GENERIC_END)
	{
		tokens->revertTo(oldIndex);
		valid = false;
	}

	args.last = tkn;

	return valid;
}

bool Parser::parseValueList(ref<std::vector<uptr<Value>>> values, bool commaOptional)
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

	//postParseException(std::make_unique<UnexpectedTokenException>(tkn, ';'));
	return false;
}

bool Parser::parseScopeEnd(uptr<ScopeStatement> stmt)
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

uptr<ScopeStatement> Parser::parseScope(std::initializer_list<ParseMethod<Statement>> pms)
{
	auto mods = parseStorageMods();

	auto tkn = tokens->current();

	if (tkn->type != TokenType::START_SCOPE)
	{
		return nullptr;
	}

	auto scope = std::make_unique<ScopeStatement>();

	tokens->consume();

	while (tokens->hasNext())
	{
		tkn = tokens->current();

		if (tkn->type == TokenType::END_SCOPE)
		{
			tokens->consume();
			break;
		}

		uptr<Statement> stmt = nullptr;

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

		scope->stmts.push_back(std::move(stmt));

	}

	return scope;
}

uptr<ParsedType> Parser::parseTypeName()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		postParseException(std::make_unique<ParseException>("Expected identifier; this is NOT a valid identifier.", tkn));
		return nullptr;
	}

	tokens->consume();

	uptr<ParsedType> type = std::make_unique<ParsedType>(tkn);

	parseGenericArgs(type->genericArgs);

	tkn = tokens->current();

	while (tokens->hasNext() && tkn->type == TokenType::START_BRACKET)
	{
		tkn = tokens->next();

		uptr<Value> len = nullptr;

		if (tkn->type != TokenType::END_BRACKET)
		{
			len = parseAnyValue();

			tkn = tokens->current();

		}

		if (tkn->type != TokenType::END_BRACKET)
		{
			//TODO complain
		}

		type->arrayDims.push_back(std::move(len));
		type->lastToken = tkn;

		tkn = tokens->next();

	}

	return type;
}

uptr<Statement> Parser::parseDecl()
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
		postParseException(std::make_unique<ParseException>("Invalid start to declaration:", tokens->current()));
		return stmt;
	}

	stmt->mods = mods;

	if (!parseSemicolon())
	{
		postParseException(std::make_unique<ParseException>("All declarations must end with a semicolon", tokens->current()));
	}

	return stmt;
}

uptr<Statement> Parser::parseImport()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "import")
	{
		return nullptr;
	}

	auto modName = tokens->next();
	
	sptr<Token> alias = nullptr;

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

	auto ret = std::make_unique<ImportStatement>(tkn);

	ret->name = modName;
	ret->alias = alias;

	return ret;
}

uptr<Statement> Parser::parseModuleDef()
{
	auto start = tokens->current();

	if (start->type != TokenType::KEYWORD || start->str != "module")
	{
		return nullptr;
	}

	auto name = tokens->next();

	return std::make_unique<ModuleStatement>(start, name);
}

uptr<Statement> Parser::parseTypedef()
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

	uptr<ParsedType> aliasedType = parseTypeName();

	auto stmt = std::make_unique<TypedefStatement>(start, name, std::move(aliasedType));
	
	return stmt;
}

//uptr<Statement> Parser::parseShader();

//uptr<Statement> Parser::parseDescriptor();

//TODO clean up architecture, make expandable
uptr<Statement> Parser::parseStruct()
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

	auto ret = std::make_unique<StructStatement>(name, isConst ? StatementType::RECORD : StatementType::STRUCT);

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

//uptr<Statement> Parser::parseClass();

uptr<Statement> Parser::parseFunction()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "def")
	{
		return nullptr;
	}

	tkn = tokens->next();

	uptr<ParsedType> type = parseTypeName();
	std::string name = tokens->current()->str;
	GenericSignature genSig;
	std::vector<sptr<Token>> gpuThreadData;

	tokens->consume();

	parseGenericSig(genSig);

	//parse GPU threading data
	if (tkn->str == "[")
	{
		parseIdentifierList(gpuThreadData);

		if (gpuThreadData.size() == 0)
		{
			postParseException(std::make_unique<ParseException>("GPU threading data is empty or has invalid values", tkn));
		}

		if (tokens->current()->str != "]")
		{
			postParseException(std::make_unique<UnexpectedTokenException>(tokens->current(), ']'));
		}

		tkn = tokens->next();

	}

	if (tkn->str != "(")
	{
		postParseException(std::make_unique<UnexpectedTokenException>(tokens->current(), '('));
		return nullptr;
	}

	//TODO parse arg list

	tkn = tokens->next();

	if (tkn->str != ")")
	{
		postParseException(std::make_unique<UnexpectedTokenException>(tokens->current(), ')'));
		return nullptr;
	}

	uptr<Statement> body = parseLogic();

	if (!body)
	{
		return nullptr;
	}
	/*
	Functionptr<Statement> func = std::make_unique<FunctionStatement>();

	func->type = type;
	func->name = name;
	func->funcBody = body;
	//TODO finish

	return func;*/
	return nullptr;
}

uptr<Statement> Parser::parseMethod()
{
	return parseAny({
		&Parser::parseFunction,
		&Parser::parseConstructor,
		&Parser::parseDestructor
		});
}

uptr<Statement> Parser::parseConstructor()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "construct")
	{
		return nullptr;
	}

	return nullptr;
}

uptr<Statement> Parser::parseDestructor()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "destruct")
	{
		return nullptr;
	}

	return nullptr;
}

//uptr<Statement> parseOp();

uptr<Statement> Parser::parseLogic()
{
	return parseAny({ &Parser::parseControl, &Parser::parseValueStmt });
}

uptr<Statement> Parser::parseControl()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	return parseAny({ &Parser::parseIf, &Parser::parseFor, &Parser::parseWhile,
		&Parser::parseDoWhile, /* parseSwitch */ });
}

uptr<Statement> Parser::parseIf()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "if")
	{
		return nullptr;
	}

	tokens->consume();

	auto parsed = std::make_unique<IfStatement>();

	parsed->condition = parseBetween("(", &Parser::parseAnyValue, ")");
	parsed->innerIf = parseScope({&Parser::parseDecl});

	if (tokens->current()->str == "else")
	{
		tokens->consume();

		auto innerElseStmt = parseLogic();

		if (innerElseStmt != nullptr)
		{
			parsed->innerElse = std::make_unique<ScopeStatement>();

			parsed->innerElse->stmts.push_back(std::move(innerElseStmt));

		}
		else
		{
			parsed->innerElse = parseScope({&Parser::parseDecl});

		}

	}

	return parsed;
}

uptr<Statement> Parser::parseFor()
{
	return nullptr;
}

uptr<Statement> Parser::parseWhile()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "while")
	{
		return nullptr;
	}

	auto cond = parseBetween("(", &Parser::parseAnyValue, ")");

	auto stmt = std::make_unique<WhileStatement>();
	
	stmt->first = tkn;
	stmt->condition = std::move(cond);
	stmt->loop = parseScope({ &Parser::parseLogic });

	return stmt;
}

uptr<Statement> Parser::parseDoWhile()
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
		return nullptr;
	}

	uptr<Value> cond = parseBetween("(", &Parser::parseAnyValue, ")");

	if (cond == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	auto ret = std::make_unique<WhileStatement>();

	ret->doWhile = true;
	ret->loop = std::move(body);
	ret->condition = std::move(cond);

	return ret;
}

uptr<Statement> Parser::parseValueStmt()
{
	auto val = parseAny({ &Parser::parseAnyFnCall, &Parser::parseAnyExpr });

	if (val == nullptr)
	{
		return nullptr;
	}

	return std::make_unique<ValueStatement>(std::move(val));
}

uptr<Value> Parser::parseAnyValue()
{
	return parseAny({ &Parser::parseAnyExpr, &Parser::parseLiteral, &Parser::parseAnyFnCall });
}

uptr<Value> Parser::parseNonExpr()
{
	auto tkn = tokens->current();

	auto v = parseAny({ &Parser::parseLiteral, &Parser::parseAnyFnCall });

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
				v = std::make_unique<VarReadValue>(tkn);
			}
			else if (tkn->str == "sign")
			{
				v = std::make_unique<UnaryValue>(tkn, Operator::SIGN, parseNonExpr());
			}
			else if (tkn->str == "unsign")
			{
				v = std::make_unique<UnaryValue>(tkn, Operator::UNSIGN, parseNonExpr());
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

			auto unary = std::make_unique<UnaryValue>();

			unary->op = uOp;
			unary->val = parseNonExpr();

			if (uOp == Operator::ABS)
			{
				if (tokens->current()->str != "|")
				{
					//TODO complain
					return nullptr;
				}

				tokens->consume();

			}

			v = std::move(unary);

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
			
			auto subA = std::make_unique<SubArrayValue>();

			subA->array = std::move(v);
			subA->index = std::move(i);
			subA->last = tokens->prev();

			v = std::move(subA);

		}
		else if (tkn->type == TokenType::PERIOD && tokens->peek()->type == TokenType::IDENTIFIER)
		{
			tkn = tokens->next();
			auto pk = tokens->peek();

			if (pk->type == TokenType::START_PAREN || pk->str == GENERIC_START)
			{
				v = parseFnCall(std::move(v));

			}
			else
			{
				auto memRead = std::make_unique<MemberReadValue>();

				memRead->target = std::move(v);
				memRead->memberName = tkn;

				v = std::move(memRead);

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

uptr<Value> Parser::parseLiteral()
{
	auto tkn = tokens->current();

	if (tkn->type == TokenType::KEYWORD)
	{
		if (tkn->str == "this")
		{
			return std::make_unique<VarReadValue>(tkn);
		}
		else if (tkn->str == "null")
		{
			return std::make_unique<NullValue>(tkn);
		}

	}

	switch (tkn->type)
	{
	case TokenType::LITERAL_INT: return std::make_unique<IntLiteralValue>(tkn);
	case TokenType::LITERAL_FLOAT: return std::make_unique<FloatLiteralValue>(tkn);
	case TokenType::LITERAL_BOOL: return std::make_unique<BoolLitValue>(tkn);
	case TokenType::LITERAL_STR: return std::make_unique<StringLitValue>(tkn);
	}

	if (tkn->str == "[")
	{
		auto arrLit = std::make_unique<ArrayLitValue>();

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

			arrLit->values.push_back(std::move(val));

		}

		return arrLit;
	}

	return nullptr;
}

uptr<Value> Parser::parseAnyExpr()
{
	return parseExpr(OP_PRECEDENCE_MAX);
}

uptr<Value> Parser::parseExpr(uint32_t precedence)
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

				auto isa = std::make_unique<IsAValue>();

				isa->val = std::move(lhs);
				isa->chkPType = parseTypeName();

				lhs = std::move(isa);

			}
			else if (tkn->str == "as")
			{
				tokens->consume();

				auto cast = std::make_unique<CastValue>();

				cast->lhs = std::move(lhs);
				cast->resultPType = parseTypeName();

				lhs = std::move(cast);

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

			//AND we made a dedicated class for it
			//Why? Because memory management, that's why
			auto set = std::make_unique<SetterValue>();

			set->lhs = std::move(lhs);
			set->op = op->second;
			set->rhs = parseAnyExpr();

			return set;
		}
		
		auto expr = std::make_unique<ExpressionValue>();

		expr->lValue = std::move(lhs);
		expr->op = op->second;
		expr->rValue = parseExpr(opWeight);

		lhs = std::move(expr);

	}

	return lhs;
}

uptr<Value> Parser::parseAnyFnCall()
{
	return parseFnCall(nullptr);
}

uptr<Value> Parser::parseFnCall(uptr<Value> start)
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	auto name = tkn;

	auto ret = std::make_unique<FnCallValue>();

	tkn = tokens->next();
	
	if (tkn->str == GENERIC_START)
	{
		if (!parseGenericArgs(ret->genArgs))
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

	ret->name = name;
	ret->target = std::move(start);

	if (!parseValueList(ret->args, false))
	{
		//TODO complain
	}

	ret->end = tokens->current();

	return ret;
}

sptr<Variable> Parser::parseLocalVar()
{
	uptr<ParsedType> type = nullptr;
	sptr<Token> name = nullptr;
	uptr<Value> initVal = nullptr;
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
		postParseException(std::make_unique<ParseException>("Invalid name for a variable:", name));
		return nullptr;
	}

	tkn = tokens->next();

	if (tkn->str == ":")
	{
		postParseException(std::make_unique<ParseException>("That's not how type hints work in Caliburn. Type hints go before the variable name, not after. So var: int x is valid. var x: int isn't.", tkn));
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
		postParseException(std::make_unique<ParseException>("All implicitly-typed variables must be manually initialized", tkn));
		return nullptr;
	}

	if (!parseSemicolon())
	{
		//TODO complain
	}

	return std::make_shared<LocalVariable>(name, std::move(type), std::move(initVal), constant);
}

sptr<Variable> Parser::parseMemberVar()
{
	bool isConst = false;
	auto start = tokens->current();

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
	uptr<Value> initVal = nullptr;

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	tokens->consume();

	if (tokens->current()->str == "=")
	{
		tokens->consume();
		initVal = parseAnyValue();

	}

	return std::make_shared<MemberVariable>(name, std::move(type), std::move(initVal), isConst);
}
