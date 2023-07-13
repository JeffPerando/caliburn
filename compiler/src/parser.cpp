

#include "parser.h"

#include "ast/ctrlstmt.h"
#include "ast/fnstmt.h"
#include "ast/generics.h"
#include "ast/modstmts.h"
#include "ast/shaderstmt.h"
#include "ast/structstmt.h"
#include "ast/typestmt.h"
#include "ast/valstmt.h"
#include "ast/values.h"
#include "ast/varstmt.h"

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
			postParseException(new_uptr<InvalidDeclException>(start));
			break;
		}

	}

	tokens = nullptr;

}

template<typename T>
uptr<T> Parser::parseAnyUnique(std::vector<ParseMethod<uptr<T>>> fns)
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
		auto parsed = fn(*this);

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
sptr<T> Parser::parseAnyShared(std::vector<ParseMethod<sptr<T>>> fns)
{
	auto current = tokens->currentIndex();

	for (auto fn : fns)
	{
		auto parsed = fn(*this);

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
T Parser::parseBetween(std::string start, ParseMethod<T> fn, std::string end)
{
	if (tokens->current()->str != start)
	{
		//TODO complain
		return nullptr;
	}

	tokens->consume();

	auto ret = fn(*this);

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

sptr<GenericSignature> Parser::parseGenericSig()
{
	auto oldIndex = tokens->currentIndex();
	auto tkn = tokens->current();
	bool valid = false;
	
	if (tkn->str != GENERIC_START)
	{
		//We return an empty signature since every generic needs one, even a blank one.
		return new_sptr<GenericSignature>();
	}

	std::vector<GenericName> names;

	bool parsingDefaults = false;

	while (tokens->hasNext())
	{
		tkn = tokens->next();

		if (tkn->type != TokenType::KEYWORD)
		{
			valid = false;
			break;
		}

		sptr<Token> t = tkn;
		sptr<Token> n = tokens->next();

		if (n->type != TokenType::IDENTIFIER)
		{
			valid = false;
			break;
		}

		GenericResult defRes;

		tkn = tokens->next();

		if (tkn->str == "=")
		{
			parsingDefaults = true;

			tokens->consume();

			auto v = parseLiteral();

			if (v != nullptr)
			{
				defRes = v;

			}
			else
			{
				auto t = parseTypeName();

				if (t == nullptr)
				{
					//TODO complain
					break;
				}

				defRes = t;

			}

			tkn = tokens->current();//parse methods should leave us at the first invalid token for that parse

		}
		else
		{
			if (parsingDefaults)
			{
				//TODO complain
				//Generics defaults must always be at the end
				//I may reconsider this rule if explicit-named generics become a thing. But that logic is too hard right now
			}

		}

		auto genName = GenericName(tkn, tokens->next(), defRes);

		names.push_back(genName);

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

	if (!valid)
	{
		return nullptr;
	}

	tokens->consume();

	auto sig = new_sptr<GenericSignature>(names);

	sig->first = tkn;
	sig->last = tkn;
	
	return sig;
}

sptr<GenericArguments> Parser::parseGenericArgs()
{
	auto oldIndex = tokens->currentIndex();
	auto tkn = tokens->current();
	bool valid = false;

	if (tkn->str != GENERIC_START)
	{
		return new_sptr<GenericArguments>();
	}

	std::vector<GenericResult> results;

	while (tokens->hasNext())
	{
		tkn = tokens->next();

		GenericResult result;

		auto v = parseLiteral();

		if (v != nullptr)
		{
			result = v;

		}
		else
		{
			auto t = parseTypeName();

			if (t == nullptr)
			{
				//TODO complain
				break;
			}

			result = t;

		}

		tkn = tokens->current();//parse methods should leave us at the first invalid token for that parse

		results.push_back(result);

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

	if (!valid)
	{
		return nullptr;
	}

	auto args = new_sptr<GenericArguments>(results);

	args->first = tkn;
	args->last = tkn;

	return args;
}

bool Parser::parseValueList(ref<std::vector<sptr<Value>>> values, bool commaOptional)
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

	//postParseException(new_uptr<UnexpectedTokenException>(tkn, ';'));
	return false;
}

bool Parser::parseScopeEnd(ref<uptr<ScopeStatement>> stmt)
{
	auto tkn = tokens->current();
	auto tknIndex = tokens->currentIndex();

	if (tkn->type != TokenType::KEYWORD)
	{
		return false;
	}

	bool valid = true;

	tokens->consume();

	auto retMode = RETURN_MODES.find(tkn->str);

	if (retMode == RETURN_MODES.end())
	{
		//TODO complain
		tokens->revertTo(tknIndex);

		return false;
	}

	tokens->consume();

	stmt->retMode = retMode->second;

	if (stmt->retMode == ReturnMode::RETURN)
	{
		stmt->retValue = parseAnyValue();
	}

	if (!parseSemicolon())
	{
		//TODO complain
		return false;
	}

	return true;
}

StmtModifiers Parser::parseStmtMods()
{
	StmtModifiers mods = {};

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

uptr<ScopeStatement> Parser::parseScope(std::vector<ParseMethod<uptr<Statement>>> pms)
{
	auto mods = parseStmtMods();

	auto tkn = tokens->current();

	if (tkn->type != TokenType::START_SCOPE)
	{
		return nullptr;
	}

	auto scope = new_uptr<ScopeStatement>();

	while (tokens->hasNext())
	{
		tkn = tokens->current();

		if (tkn->type == TokenType::END_SCOPE)
		{
			tokens->consume();
			break;
		}
		else if (parseScopeEnd(scope))
		{
			if (tokens->current()->type == TokenType::END_SCOPE)
			{
				tokens->consume();
				return scope;
			}
			
			//TODO complain

		}

		uptr<Statement> stmt = nullptr;

		for (auto& pm : pms)
		{
			stmt = pm(*this);

			if (stmt != nullptr)
			{
				break;
			}

		}

		if (stmt == nullptr)
		{
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

sptr<ParsedType> Parser::parseTypeName()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		postParseException(new_uptr<ParseException>("Expected identifier; this is NOT a valid identifier.", tkn));
		return nullptr;
	}

	tokens->consume();

	sptr<ParsedType> type = new_sptr<ParsedType>(tkn);

	type->genericArgs = parseGenericArgs();

	tkn = tokens->current();

	while (tokens->hasNext() && tkn->type == TokenType::START_BRACKET)
	{
		tkn = tokens->next();

		sptr<Value> len = nullptr;

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

uptr<Statement> Parser::parseDecl()
{
	auto mods = parseStmtMods();

	std::vector<ParseMethod<uptr<Statement>>> methods = {
		&Parser::parseImport,
		&Parser::parseModuleDef,
		&Parser::parseTypedef,
		&Parser::parseFunction,
		//&Parser::parseShader,
		//&Parser::parseClass,
		//&Parser::parseStruct,
		&Parser::parseLogic
	};

	uptr<Statement> stmt = parseAnyUnique(methods);

	if (stmt == nullptr)
	{
		postParseException(new_uptr<ParseException>("Invalid start to declaration:", tokens->current()));
		return stmt;
	}

	stmt->mods = mods;

	if (!parseSemicolon())
	{
		postParseException(new_uptr<ParseException>("All declarations must end with a semicolon", tokens->current()));
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

	auto ret = new_uptr<ImportStatement>(tkn);

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

	return new_uptr<ModuleStatement>(start, name);
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

	sptr<ParsedType> aliasedType = parseTypeName();

	auto stmt = new_uptr<TypedefStatement>(start, name, aliasedType);
	
	return stmt;
}

uptr<Statement> Parser::parseShader()
{
	auto tkn = tokens->next();

	if (tkn->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (tkn->str != "shader")
	{
		return nullptr;
	}

	auto shader = new_uptr<ShaderStatement>();

	shader->first = tkn;
	shader->name = tokens->next();

	tkn = tokens->next();

	if (tkn->type == TokenType::START_PAREN)
	{
		tokens->consume();

		while (tokens->hasNext())
		{
			tkn = tokens->current();

			auto type = parseTypeName();
			
			if (type == nullptr)
			{
				//TODO complain
				break;
			}

			auto name = tokens->current();

			if (name->type != TokenType::IDENTIFIER)
			{
				//TODO complain
				break;
			}

			tkn = tokens->next();

			shader->descriptors.push_back(std::pair(type, name));

			if (tkn->type == TokenType::COMMA)
			{
				continue;
			}
			else if (tkn->type == TokenType::END_PAREN)
			{
				tkn = tokens->next();
				break;
			}
			else
			{
				//TODO complain
				break;
			}

		}

		if (shader->descriptors.empty())
		{
			//TODO complain?
		}

	}

	if (tkn->type != TokenType::START_SCOPE)
	{
		//TODO complain
		return shader;
	}

	while (tokens->hasNext())
	{
		auto stage = parseFunction();

		auto fn = (ptr<Function>)stage.get();

		//if (fn->)

		//shader->stages.push_back

	}

	return shader;
}

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

	auto ret = new_uptr<StructStatement>(name, isConst ? StatementType::RECORD : StatementType::STRUCT);

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
	auto first = tokens->current();

	if (first->type != TokenType::KEYWORD || first->str != "def")
	{
		return nullptr;
	}

	auto tkn = tokens->next();

	std::vector<sptr<Token>> gpuThreadData;
	//parse GPU threading data
	if (tkn->str == "[")
	{
		parseIdentifierList(gpuThreadData);

		if (gpuThreadData.size() == 0)
		{
			postParseException(new_uptr<ParseException>("GPU threading data is empty or has invalid values", tkn));
		}

		if (tokens->current()->str != "]")
		{
			postParseException(new_uptr<UnexpectedTokenException>(tokens->current(), ']'));
		}

		tkn = tokens->next();

	}

	auto name = tokens->current();
	tokens->consume();

	auto genSig = parseGenericSig();

	if (genSig == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	if (tkn->str != "(")
	{
		postParseException(new_uptr<UnexpectedTokenException>(tokens->current(), '('));
		return nullptr;
	}

	tkn = tokens->next();

	std::vector<sptr<FnArgVariable>> args;

	uint32_t argIndex = 0;

	while (tokens->hasNext())
	{
		auto argType = parseTypeName();

		if (argType == nullptr)
		{
			break;
		}

		auto argName = tokens->current();

		if (argName->type != TokenType::IDENTIFIER)
		{
			//TODO complain
			break;
		}

		auto arg = new_sptr<FnArgVariable>(argIndex++);

		arg->start = argType->firstTkn();
		arg->nameTkn = argName;
		arg->typeHint = argType;
		
		args.push_back(arg);

	}

	if (tkn->str != ")")
	{
		postParseException(new_uptr<UnexpectedTokenException>(tokens->current(), ')'));
		return nullptr;
	}

	tkn = tokens->next();

	sptr<ParsedType> type = nullptr;

	if (tkn->type == TokenType::ARROW)
	{
		tokens->consume();
		type = parseTypeName();
	}

	auto body = parseScope({&Parser::parseLogic});

	if (!body)
	{
		return nullptr;
	}
	
	auto func = new_uptr<FunctionStatement>();

	func->first = first;
	func->name = name;
	func->genSig = genSig;
	func->retType = type;
	func->args = args;
	func->body = std::move(body);

	return func;
}

uptr<Statement> Parser::parseMethod()
{
	std::vector<ParseMethod<uptr<Statement>>> methods = {
		&Parser::parseFunction,
		&Parser::parseConstructor,
		&Parser::parseDestructor
	};

	return parseAnyUnique(methods);
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
	std::vector<ParseMethod<uptr<Statement>>> methods = {
		&Parser::parseControl,
		&Parser::parseValueStmt
	};

	return parseAnyUnique(methods);
}

uptr<Statement> Parser::parseControl()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD)
	{
		return nullptr;
	}
	
	std::vector<ParseMethod<uptr<Statement>>> methods = {
		&Parser::parseIf,
		&Parser::parseFor,
		&Parser::parseWhile,
		&Parser::parseDoWhile,
		//&Parser::parseSwitch
	};

	return parseAnyUnique(methods);
}

uptr<Statement> Parser::parseIf()
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "if")
	{
		return nullptr;
	}

	tokens->consume();

	auto parsed = new_uptr<IfStatement>();

	parsed->condition = parseBetween("(", ParseMethod<sptr<Value>>(&Parser::parseAnyValue), ")");
	parsed->innerIf = parseScope({&Parser::parseDecl});

	if (tokens->current()->str == "else")
	{
		tokens->consume();

		auto innerElseStmt = parseLogic();

		if (innerElseStmt != nullptr)
		{
			parsed->innerElse = new_uptr<ScopeStatement>();

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

	auto cond = parseBetween("(", ParseMethod<sptr<Value>>(&Parser::parseAnyValue), ")");

	auto stmt = new_uptr<WhileStatement>();
	
	stmt->first = tkn;
	stmt->condition = cond;
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

	auto cond = parseBetween("(", ParseMethod<sptr<Value>>(&Parser::parseAnyValue), ")");

	if (cond == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	auto ret = new_uptr<WhileStatement>();

	ret->doWhile = true;
	ret->loop = std::move(body);
	ret->condition = cond;

	return ret;
}

uptr<Statement> Parser::parseValueStmt()
{
	std::vector<ParseMethod<sptr<Value>>> methods = {
		&Parser::parseAnyFnCall,
		&Parser::parseAnyExpr
	};

	auto val = parseAnyShared(methods);

	if (val == nullptr)
	{
		return nullptr;
	}

	return new_uptr<ValueStatement>(val);
}

uptr<Statement> Parser::parseLocalVarStmt()
{
	sptr<ParsedType> type = nullptr;
	sptr<Token> name = nullptr;
	sptr<Value> initVal = nullptr;
	bool isConst = false;

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
		isConst = true;
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

	auto ret = new_uptr<LocalVarStatement>();

	parseIdentifierList(ret->names);

	tkn = tokens->current();

	if (tkn->str == ":")
	{
		postParseException(new_uptr<ParseException>("That's not how type hints work in Caliburn. Type hints go before the variable name, not after. So var: int x is valid. var x: int isn't.", tkn));
		return nullptr;
	}

	if (tkn->str == "=")
	{
		tokens->consume();
		initVal = parseAnyValue();
		
	}
	else if (!type)
	{
		postParseException(new_uptr<ParseException>("All implicitly-typed variables must be manually initialized", tkn));
		return nullptr;
	}

	if (!parseSemicolon())
	{
		//TODO complain
	}

	ret->isConst = isConst;

	if (initVal != nullptr)
	{
		ret->initialValue = initVal;

	}
	
	if (type != nullptr)
	{
		ret->typeHint = type;
	}
	
	return ret;
}

sptr<Value> Parser::parseAnyValue()
{
	std::vector<ParseMethod<sptr<Value>>> methods = {
		&Parser::parseAnyExpr,
		&Parser::parseLiteral,
		&Parser::parseAnyFnCall
	};

	return parseAnyShared(methods);
}

sptr<Value> Parser::parseNonExpr()
{
	auto tkn = tokens->current();

	std::vector<ParseMethod<sptr<Value>>> methods = { &Parser::parseLiteral, &Parser::parseAnyFnCall };

	auto v = parseAnyShared(methods);

	if (v == nullptr)
	{
		if (tkn->type == TokenType::START_PAREN)
		{
			v = parseBetween("(", ParseMethod<sptr<Value>>(&Parser::parseNonExpr), ")");
		}
		else if (tkn->type == TokenType::KEYWORD)
		{
			tokens->consume();

			if (tkn->str == "this")
			{
				v = new_sptr<VarReadValue>(tkn);
			}
			else if (tkn->str == "sign")
			{
				v = new_sptr<SignValue>(tkn, parseNonExpr());
			}
			else if (tkn->str == "unsign")
			{
				v = new_sptr<UnsignValue>(tkn, parseNonExpr());
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

			auto unary = new_sptr<UnaryValue>();

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
			auto i = parseBetween("[", ParseMethod<sptr<Value>>(&Parser::parseAnyValue), "]");//enables for expressions inside of array access
			
			auto subA = new_sptr<SubArrayValue>();

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
				auto memRead = new_sptr<MemberReadValue>();

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

	tkn = tokens->current();

	while (tkn->type == TokenType::KEYWORD)
	{
		if (tkn->str == "is")
		{
			tokens->consume();

			auto isa = new_sptr<IsAValue>();

			isa->val = v;
			isa->chkType = parseTypeName();

			v = isa;

		}
		else if (tkn->str == "as")
		{
			tokens->consume();

			auto cast = new_sptr<CastValue>();

			cast->lhs = v;
			cast->castTarget = parseTypeName();

			v = cast;

		}
		else
		{
			break;
		}

	}

	return v;
}

sptr<Value> Parser::parseLiteral()
{
	auto tkn = tokens->current();

	if (tkn->type == TokenType::KEYWORD)
	{
		if (tkn->str == "this")
		{
			return new_sptr<VarReadValue>(tkn);
		}
		else if (tkn->str == "null")
		{
			return new_sptr<NullValue>(tkn);
		}

	}

	switch (tkn->type)
	{
	case TokenType::LITERAL_INT: return new_sptr<IntLiteralValue>(tkn);
	case TokenType::LITERAL_FLOAT: return new_sptr<FloatLiteralValue>(tkn);
	case TokenType::LITERAL_BOOL: return new_sptr<BoolLitValue>(tkn);
	case TokenType::LITERAL_STR: return new_sptr<StringLitValue>(tkn);
	}

	if (tkn->str == "[")
	{
		auto arrLit = new_sptr<ArrayLitValue>();

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

			arrLit->values.push_back(parseAnyValue());

		}

		return arrLit;
	}

	return nullptr;
}

sptr<Value> Parser::parseAnyExpr()
{
	return parseExpr(OP_PRECEDENCE_MAX);
}

sptr<Value> Parser::parseExpr(uint32_t precedence)
{
	auto lhs = parseNonExpr();

	while (tokens->hasNext())
	{
		std::vector<sptr<Token>> opTkns;

		/*
		Caliburn operator tokens consist of individual characters. So += is two tokens: '+' and '='

		So we gather the tokens into a vector for later processing
		*/
		for (size_t off = 0; tokens->currentIndex() + off < tokens->length(); ++off)
		{
			auto offTkn = tokens->peek(off);

			if (offTkn->type == TokenType::OPERATOR)
			{
				opTkns.push_back(offTkn);
				continue;
			}

			break;
		}

		if (opTkns.empty())
		{
			break;
		}

		/*
		Now we take the vector of tokens from earlier, and we turn them into a single string.

		That string is then compared against the canonical map of operators. If a particular string
		is invalid, we remove the last token and repeat the process until we find a valid operator.
		*/
		while (!opTkns.empty())
		{
			std::stringstream ss;

			for (auto const& tkn : opTkns)
			{
				ss << tkn->str;
			}

			auto found = INFIX_OPS.find(ss.str());

			if (found == INFIX_OPS.end())
			{
				opTkns.pop_back();
				continue;
			}

			Operator op = found->second;

			if (op == Operator::UNKNOWN)
			{
				continue;
			}

			auto opWeight = OP_PRECEDENCE.at(op);

			if (opWeight > precedence)
			{
				return lhs;
			}

			tokens->consume(opTkns.size());

			auto tkn = tokens->current();

			//Yes, we use *this* code to parse setters.
			if (tkn->str == "=")
			{
				if (OP_CATEGORIES.find(op)->second == OpCategory::LOGICAL)
				{
					//TODO complain
					return nullptr;
				}

				tokens->consume();

				//AND we made a dedicated class for it
				//Why? Because memory management, that's why
				auto set = new_sptr<SetterValue>();

				set->lhs = lhs;
				set->op = op;
				set->rhs = parseAnyExpr();

				return set;
			}

			auto expr = new_sptr<ExpressionValue>();

			auto rhs = parseExpr(opWeight);//TODO suppress errors here

			if (rhs == nullptr)
			{
				tokens->rewind(opTkns.size());
				continue;
			}

			expr->lValue = lhs;
			expr->op = op;
			expr->rValue = rhs;

			lhs = expr;

			break;
		}

	}

	if (lhs->vType != ValueType::EXPRESSION)
	{
		//TODO complain
	}

	return lhs;
}

sptr<Value> Parser::parseAnyFnCall()
{
	return parseFnCall(nullptr);
}

sptr<Value> Parser::parseFnCall(sptr<Value> start)
{
	auto tkn = tokens->current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	auto const& name = tkn;

	auto ret = new_sptr<FnCallValue>();

	tokens->consume();
	
	ret->genArgs = parseGenericArgs();

	if (ret->genArgs == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	tkn = tokens->current();

	if (tkn->type != TokenType::START_PAREN)
	{
		return nullptr;
	}

	tokens->consume();

	ret->name = name;
	ret->target = start;

	if (!parseValueList(ret->args, false))
	{
		//TODO complain
	}

	ret->end = tokens->current();

	return ret;
}

sptr<Variable> Parser::parseMemberVar()
{
	auto mods = parseStmtMods();

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
	sptr<Value> initVal = nullptr;

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

	auto v = new_sptr<MemberVariable>();

	v->mods = mods;
	v->start = start;
	v->nameTkn = name;
	v->typeHint = type;
	v->initValue = initVal;
	v->isConst = isConst;
	
	return v;
}
