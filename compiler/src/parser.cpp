
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

std::vector<uptr<Statement>> Parser::parse()
{
	std::vector<uptr<Statement>> ast;

	while (tokens.hasNext())
	{
		auto start = tokens.current();
		auto finished = parseDecl();

		if (finished)
		{
			ast.push_back(std::move(finished));
		}
		else
		{
			//postParseException(new_uptr<InvalidDeclException>(start));
			break;
		}

	}

	return ast;
}

template<typename T>
T Parser::parseAny(std::vector<ParseMethod<T>> fns)
{
	/*
	this code is really smart, and stupid. at the same time.
	it takes a bunch of method/function pointers, puts them into a list,
	then calls them all. if it finds one that doesn't return nullptr,
	it returns that result.
	The alternative was very repetitive code.
	*/
	auto current = tokens.currentIndex();

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
			tokens.revertTo(current);

		}

	}

	return nullptr;
}

template<typename T>
T Parser::parseBetween(std::string start, ParseMethod<T> fn, std::string end)
{
	if (tokens.current()->str != start)
	{
		//TODO complain
		return nullptr;
	}

	tokens.consume();

	auto ret = fn(*this);

	if (tokens.current()->str != end)
	{
		//TODO complain
		return ret;
	}

	tokens.consume();

	return ret;
}

void Parser::parseAnyBetween(std::string start, std::function<void()> fn, std::string end)
{
	if (tokens.current()->str != start)
	{
		//TODO complain
		return;
	}

	tokens.consume();

	fn();

	if (tokens.current()->str != end)
	{
		//TODO complain
		return;
	}

	tokens.consume();

}

std::vector<sptr<Token>> Parser::parseIdentifierList()
{
	std::vector<sptr<Token>> ids;

	while (tokens.hasNext())
	{
		auto tkn = tokens.current();

		if (tkn->type != TokenType::IDENTIFIER)
		{
			break;
		}

		ids.push_back(tkn);
		tkn = tokens.next();

		if (tkn->type != TokenType::COMMA)
		{
			break;
		}

		tokens.consume();

	}

	return ids;
}

sptr<GenericSignature> Parser::parseGenericSig()
{
	auto oldIndex = tokens.currentIndex();
	auto tkn = tokens.current();
	bool valid = false;
	
	if (tkn->str != GENERIC_START)
	{
		//We return an empty signature since every generic needs one, even a blank one.
		return new_sptr<GenericSignature>();
	}

	std::vector<GenericName> names;

	bool parsingDefaults = false;

	while (tokens.hasNext())
	{
		tkn = tokens.next();

		if (tkn->type != TokenType::KEYWORD)
		{
			valid = false;
			break;
		}

		sptr<Token> t = tkn;
		sptr<Token> n = tokens.next();

		if (n->type != TokenType::IDENTIFIER)
		{
			valid = false;
			break;
		}

		GenericResult defRes;

		tkn = tokens.next();

		if (tkn->str == "=")
		{
			parsingDefaults = true;

			tokens.consume();

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

			tkn = tokens.current();//parse methods should leave us at the first invalid token for that parse

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

		auto genName = GenericName(tkn, tokens.next(), defRes);

		names.push_back(genName);

		if (tkn->str != ",")
		{
			break;
		}

		tokens.consume();

	}

	tkn = tokens.current();

	if (tkn->str != GENERIC_END)
	{
		tokens.revertTo(oldIndex);
		valid = false;
	}

	if (!valid)
	{
		return nullptr;
	}

	tokens.consume();

	auto sig = new_sptr<GenericSignature>(names);

	sig->first = tkn;
	sig->last = tkn;
	
	return sig;
}

sptr<GenericArguments> Parser::parseGenericArgs()
{
	auto oldIndex = tokens.currentIndex();
	auto tkn = tokens.current();
	bool valid = false;

	if (tkn->str != GENERIC_START)
	{
		return new_sptr<GenericArguments>();
	}

	std::vector<GenericResult> results;

	while (tokens.hasNext())
	{
		tkn = tokens.next();

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

		tkn = tokens.current();//parse methods should leave us at the first invalid token for that parse

		results.push_back(result);

		if (tkn->str != ",")
		{
			break;
		}

		tokens.consume();

	}

	tkn = tokens.current();

	if (tkn->str != GENERIC_END)
	{
		tokens.revertTo(oldIndex);
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
	while (tokens.hasNext())
	{
		values.push_back(parseAnyValue());

		if (tokens.current()->str == ",")
		{
			tokens.consume();
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
	auto tkn = tokens.current();

	if (tkn->type == TokenType::END)
	{
		tokens.consume();
		return true;
	}

	//postParseException(new_uptr<UnexpectedTokenException>(tkn, ';'));
	return false;
}

bool Parser::parseScopeEnd(ref<uptr<ScopeStatement>> stmt)
{
	auto tkn = tokens.current();
	auto tknIndex = tokens.currentIndex();

	if (tkn->type != TokenType::KEYWORD)
	{
		return false;
	}

	bool valid = true;

	tokens.consume();

	auto retMode = RETURN_MODES.find(tkn->str);

	if (retMode == RETURN_MODES.end())
	{
		//TODO complain
		tokens.revertTo(tknIndex);

		return false;
	}

	stmt->retMode = retMode->second;

	if (stmt->retMode == ReturnMode::RETURN)
	{
		stmt->retValue = parseAnyValue();
	}

	if (!parseSemicolon())
	{
		//TODO complain
	}

	return true;
}

StmtModifiers Parser::parseStmtMods()
{
	StmtModifiers mods = {};

	while (tokens.hasNext())
	{
		auto tkn = tokens.current();

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

		tokens.consume();

	}

	return mods;
}

uptr<ScopeStatement> Parser::parseScope(std::vector<ParseMethod<uptr<Statement>>> pms)
{
	auto mods = parseStmtMods();

	auto tkn = tokens.current();

	if (tkn->type != TokenType::START_SCOPE)
	{
		return nullptr;
	}

	tokens.consume();

	auto scope = new_uptr<ScopeStatement>();

	while (tokens.hasNext())
	{
		tkn = tokens.current();

		if (tkn->type == TokenType::END_SCOPE)
		{
			tokens.consume();
			break;
		}
		else if (parseScopeEnd(scope))
		{
			if (!parseSemicolon())
			{
				//TODO complain
			}
			
			return scope;
		}

		for (auto& pm : pms)
		{
			if (auto stmt = pm(*this))
			{
				if (!parseSemicolon())
				{
					//TODO complain but continue parsing
				}

				scope->stmts.push_back(std::move(stmt));

				break;
			}

		}

	}

	return scope;
}

sptr<ParsedType> Parser::parseTypeName()
{
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::IDENTIFIER && first->str != "dynamic")
	{
		//TODO complain
		return nullptr;
	}

	tokens.consume();

	sptr<ParsedType> type = new_sptr<ParsedType>(first);

	type->genericArgs = parseGenericArgs();

	auto tkn = tokens.current();

	while (tokens.hasNext() && tkn->type == TokenType::START_BRACKET)
	{
		tkn = tokens.next();

		auto arrayType = new_sptr<ParsedType>("array");

		auto gArgs = new_sptr<GenericArguments>();
		gArgs->args.push_back(type);

		if (tokens.current()->type != TokenType::END_BRACKET)
		{
			auto len = parseAnyValue();

			if (!len->isCompileTimeConst())
			{
				//TODO complain
			}

			gArgs->args.push_back(len);
		}

		if (tokens.current()->type != TokenType::END_BRACKET)
		{
			//TODO complain
		}

		type = arrayType;
		type->lastToken = tkn;

		tkn = tokens.next();

	}

	return type;
}

uptr<Statement> Parser::parseDecl()
{
	auto annotations = parseAllAnnotations();
	auto mods = parseStmtMods();

	std::vector<ParseMethod<uptr<Statement>>> methods = {
		&Parser::parseImport,
		&Parser::parseModuleDef,
		&Parser::parseGlobalVarStmt,
		&Parser::parseTypedef,
		&Parser::parseFnStmt,
		&Parser::parseShader,
		&Parser::parseStruct,
		&Parser::parseIf
	};

	uptr<Statement> stmt = parseAny(methods);

	if (stmt == nullptr)
	{
		//TODO complain
		return stmt;
	}

	for (auto& a : annotations)
	{
		stmt->annotations.emplace(a->name->str, std::move(a));
	}

	stmt->mods = mods;

	if (!parseSemicolon())
	{
		//TODO complain
	}

	return stmt;
}

uptr<Statement> Parser::parseImport()
{
	auto tkn = tokens.current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "import")
	{
		return nullptr;
	}

	auto modName = tokens.next();
	
	sptr<Token> alias = nullptr;

	tkn = tokens.next();

	if (tkn->type == TokenType::KEYWORD &&
		tkn->str == "as")
	{
		if (tokens.peek()->type != TokenType::IDENTIFIER)
		{
			//TODO complain
		}

		alias = tokens.next();

		tokens.consume();

	}

	auto ret = new_uptr<ImportStatement>(tkn);

	ret->name = modName;
	ret->alias = alias;

	return ret;
}

uptr<Statement> Parser::parseModuleDef()
{
	auto start = tokens.current();

	if (start->type != TokenType::KEYWORD || start->str != "module")
	{
		return nullptr;
	}

	auto name = tokens.next();

	return new_uptr<ModuleStatement>(start, name);
}

uptr<Statement> Parser::parseTypedef()
{
	auto start = tokens.current();

	if (start->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (start->str == "strong")
	{
		tokens.consume();
	}

	if (tokens.current()->str != "type")
	{
		return nullptr;
	}

	if (tokens.next()->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	auto name = tokens.current();

	if (tokens.next()->str != "=")
	{
		return nullptr;
	}

	tokens.consume();

	sptr<ParsedType> aliasedType = parseTypeName();

	auto stmt = new_uptr<TypedefStatement>(start, name, aliasedType);
	
	return stmt;
}

uptr<Statement> Parser::parseShader()
{
	auto first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (first->str != "shader")
	{
		return nullptr;
	}

	auto shader = new_uptr<ShaderStatement>();

	shader->first = first;
	shader->name = tokens.next();

	auto tkn = tokens.next();

	if (tkn->type == TokenType::START_PAREN)
	{
		tokens.consume();

		while (tokens.hasNext())
		{
			if (tkn->type == TokenType::END_PAREN)
			{
				tkn = tokens.next();
				break;
			}

			tkn = tokens.current();

			auto type = parseTypeName();
			
			if (type == nullptr)
			{
				//TODO complain
				break;
			}

			auto name = tokens.current();

			if (name->type != TokenType::IDENTIFIER)
			{
				//TODO complain
				break;
			}

			tkn = tokens.next();

			shader->descriptors.push_back(std::pair(type, name));

			if (tkn->type == TokenType::COMMA)
			{
				continue;
			}
			else if (tkn->type == TokenType::END_PAREN)
			{
				tkn = tokens.next();
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

	while (tokens.hasNext())
	{
		tkn = tokens.next();

		if (tkn->type == TokenType::END_SCOPE)
		{
			break;
		}

		if (auto stageFn = parseFnLike())
		{
			shader->stages.push_back(new_uptr<ShaderStage>(stageFn, shader->name));

		}
		else
		{
			auto ios = parseMemberVarLike();

			if (ios.empty())
			{
				//TODO complain
			}

			for (auto const& ioData : ios)
			{
				if (ioData->isConst)
				{
					//TODO complain
				}

				shader->ioVars.push_back(new_sptr<ShaderIOVariable>(*ioData));

			}

		}
		/*
		if (!parseSemicolon())
		{
			//TODO complain
		}
		*/
	}

	return shader;
}

uptr<Statement> Parser::parseStruct()
{
	auto first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	auto stmtType = StatementType::STRUCT;

	if (first->str == "record")
	{
		stmtType = StatementType::RECORD;
	}
	else if (first->str == "class")
	{
		stmtType = StatementType::CLASS;
	}
	else if (first->str != "struct")
	{
		return nullptr;
	}

	auto name = tokens.next();

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
	}

	auto stmt = new_uptr<StructStatement>(name, stmtType);

	stmt->first = first;

	auto tkn = tokens.next();

	if (tkn->str != "{")
	{
		//TODO complain
	}

	while (tokens.hasNext())
	{
		tkn = tokens.next();

		if (tkn->str == "}")
		{
			stmt->last = tkn;
			tokens.consume();
			break;
		}

		if (auto v = parseMemberVar())
		{
			if (stmtType == StatementType::RECORD)
			{
				v->isConst = true;
			}

			stmt->members.emplace(v->nameTkn->str, v);

		}
		else if (auto fn = parseMethod())
		{
			//TODO check type

			stmt->members.emplace(fn->name->str, fn);

		}

		if (!parseSemicolon())
		{
			//TODO complain
		}

	}

	return stmt;
}

uptr<Statement> Parser::parseFnStmt()
{
	auto fnLike = parseFnLike();

	if (fnLike == nullptr)
	{
		return nullptr;
	}

	auto stmt = new_uptr<FunctionStatement>();

	stmt->first = fnLike->first;
	stmt->name = fnLike->name;
	stmt->fn = new_sptr<Function>(*fnLike);

	return stmt;
}

uptr<ParsedFn> Parser::parseFnLike()
{
	auto& first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (!std::binary_search(FN_STARTS.begin(), FN_STARTS.end(), first->str))
	{
		return nullptr;
	}

	tokens.consume();

	auto fn = new_uptr<ParsedFn>();

	fn->first = first;
	fn->type = FN_TYPES.at(first->str);

	if (fn->type == FnType::FUNCTION)
	{
		if (tokens.current()->str == "[")
		{
			parseAnyBetween("[", lambda(){
				fn->invokeDims = parseIdentifierList();
			}, "]");

			if (fn->invokeDims.size() == 0)
			{
				//TODO complain
				//postParseException(new_uptr<ParseException>("GPU threading data is empty or has invalid values", tkn));
			}

			tokens.consume();

		}

	}

	//TODO reconsider
	if (fn->type == FnType::CONSTRUCTOR || fn->type == FnType::DESTRUCTOR)
	{
		fn->name = first;
	}
	else
	{
		fn->name = tokens.current();
		tokens.consume();

	}

	if (fn->type == FnType::FUNCTION)
	{
		fn->genSig = parseGenericSig();

		if (fn->genSig == nullptr)
		{
			//TODO complain
			return nullptr;
		}

	}
	
	parseAnyBetween("(", lambda(){
		fn->args = parseFnArgs();
	}, ")");

	sptr<ParsedType> retType = nullptr;

	if (tokens.current()->type == TokenType::ARROW)
	{
		tokens.consume();
		retType = parseTypeName();

		if (retType == nullptr)
		{
			//TODO complain
			return nullptr;
		}

	}
	else
	{
		retType = new_sptr<ParsedType>("void");
	}

	fn->retType = retType;
	
	auto scopeStart = tokens.current();
	
	if (scopeStart->type != TokenType::START_SCOPE)
	{
		auto e = errors->err("Invalid start to scope", scopeStart, nullptr);

	}

	fn->code = parseScope({&Parser::parseLogic});

	return fn;
}

sptr<Function> Parser::parseMethod()
{
	if (auto fnData = parseFnLike())
	{
		switch (fnData->type)
		{
		case FnType::FUNCTION: return new_sptr<Method>(*fnData);
		case FnType::CONSTRUCTOR: break;
		case FnType::DESTRUCTOR: break;
		case FnType::CONVERTER: break;
		case FnType::OP_OVERLOAD: break;
		default: break;//TODO complain
		}

	}

	return nullptr;
}

uptr<Statement> Parser::parseLogic()
{
	std::vector<ParseMethod<uptr<Statement>>> methods = {
		&Parser::parseControl,
		&Parser::parseValueStmt
	};

	return parseAny(methods);
}

uptr<Statement> Parser::parseControl()
{
	auto start = tokens.currentIndex();

	auto as = parseAllAnnotations();

	auto const& tkn = tokens.current();

	if (tkn->type != TokenType::KEYWORD)
	{
		tokens.revertTo(start);
		return nullptr;
	}
	
	std::vector<ParseMethod<uptr<Statement>>> methods = {
		&Parser::parseIf,
		//&Parser::parseFor,
		&Parser::parseWhile,
		&Parser::parseDoWhile,
		//&Parser::parseSwitch
	};

	auto ctrl = parseAny(methods);

	if (ctrl == nullptr)
	{
		tokens.revertTo(start);
		return nullptr;
	}

	if (!as.empty())
	{
		for (auto& a : as)
		{
			ctrl->annotations.emplace(a->name->str, std::move(a));
		}

	}

	return ctrl;
}

uptr<Statement> Parser::parseIf()
{
	auto const& first = tokens.current();

	if (first->type != TokenType::KEYWORD || first->str != "if")
	{
		return nullptr;
	}

	tokens.consume();

	auto stmt = new_uptr<IfStatement>();

	stmt->first = first;
	stmt->condition = parseBetween("(", ParseMethod<sptr<Value>>(&Parser::parseAnyValue), ")");
	stmt->innerIf = parseScope({&Parser::parseDecl});

	if (tokens.current()->str == "else")
	{
		tokens.consume();

		auto innerElseStmt = parseLogic();

		if (innerElseStmt != nullptr)
		{
			stmt->innerElse = new_uptr<ScopeStatement>();

			stmt->innerElse->stmts.push_back(std::move(innerElseStmt));

		}
		else
		{
			stmt->innerElse = parseScope({&Parser::parseDecl});

		}

	}

	return stmt;
}

uptr<Statement> Parser::parseFor() //FIXME
{
	return nullptr;
}

uptr<Statement> Parser::parseWhile()
{
	auto const& first = tokens.current();

	if (first->type != TokenType::KEYWORD || first->str != "while")
	{
		return nullptr;
	}

	auto cond = parseBetween("(", ParseMethod<sptr<Value>>(&Parser::parseAnyValue), ")");

	auto stmt = new_uptr<WhileStatement>();
	
	stmt->first = first;
	stmt->condition = cond;
	stmt->loop = parseScope({ &Parser::parseLogic });

	return stmt;
}

uptr<Statement> Parser::parseDoWhile()
{
	auto tkn = tokens.current();

	if (tkn->type != TokenType::KEYWORD || tkn->str != "do")
	{
		return nullptr;
	}

	tokens.consume();

	auto body = parseScope({&Parser::parseLogic});

	if (tokens.current()->type != TokenType::KEYWORD || tokens.current()->str != "while")
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
		//&Parser::parseAnyFnCall,
		&Parser::parseAnyExpr
	};

	auto val = parseAny(methods);

	if (val == nullptr)
	{
		return nullptr;
	}

	return new_uptr<ValueStatement>(val);
}

uptr<Statement> Parser::parseGlobalVarStmt()
{
	auto first = tokens.current();

	if (auto v = parseGlobalVar())
	{
		auto stmt = new_uptr<VariableStatement>();

		stmt->first = first;

		stmt->vars.push_back(v);

		return stmt;
	}

	return nullptr;
}

uptr<Statement> Parser::parseLocalVarStmt()
{
	auto first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	auto stmt = new_uptr<VariableStatement>();

	stmt->first = first;
	stmt->vars = parseLocalVars();

	if (stmt->vars.empty())
	{
		//TODO complain?
		return nullptr;
	}

	return stmt;
}

sptr<Value> Parser::parseAnyValue()
{
	std::vector<ParseMethod<sptr<Value>>> methods = {
		&Parser::parseAnyExpr,//this will call the other two and look for an expression
		&Parser::parseLiteral,
		&Parser::parseAnyFnCall,
	};

	return parseAny(methods);
}

sptr<Value> Parser::parseNonExpr()
{
	std::vector<ParseMethod<sptr<Value>>> initParsers =
		{ &Parser::parseLiteral, &Parser::parseAnyFnCall, &Parser::parseAnyAccess };

	auto v = parseAny(initParsers);

	if (v == nullptr)
	{
		auto const& first = tokens.current();

		if (first->type == TokenType::START_PAREN)
		{
			tokens.consume();
			v = parseBetween("(", ParseMethod<sptr<Value>>(&Parser::parseAnyExpr), ")");
		}
		else if (first->type == TokenType::KEYWORD)
		{
			tokens.consume();

			if (first->str == "sign")
			{
				v = new_sptr<SignValue>(first, parseNonExpr());
			}
			else if (first->str == "unsign")
			{
				v = new_sptr<UnsignValue>(first, parseNonExpr());
			}
			else
			{
				tokens.rewind();
				return nullptr;
			}

		}
		else if (first->type == TokenType::OPERATOR)
		{
			Operator uOp = Operator::UNKNOWN;
			
			switch (first->str[0])
			{
				case '|': uOp = Operator::ABS; break;
				case '-': uOp = Operator::NEG; break;
				case '~': uOp = Operator::BIT_NEG; break;
				case '!': uOp = Operator::BOOL_NOT; break;
				default: {
					return nullptr;
				};
			}

			tokens.consume();

			auto unary = new_sptr<UnaryValue>();

			unary->op = uOp;
			unary->val = parseAnyExpr();

			if (uOp == Operator::ABS)
			{
				if (tokens.current()->str != "|")
				{
					//TODO complain
					return nullptr;
				}

				unary->end = tokens.current();

				tokens.consume();

			}

			v = unary;

		}
		
		if (v == nullptr)
		{
			return nullptr;
		}

	}

	while (tokens.hasNext())
	{
		auto const& tkn = tokens.current();

		if (tkn->type == TokenType::PERIOD && tokens.peek()->type == TokenType::IDENTIFIER)
		{
			tokens.consume();
			v = parseAccess(v);
		}
		else if (tkn->type == TokenType::START_BRACKET)
		{
			auto i = parseBetween("[", ParseMethod<sptr<Value>>(&Parser::parseAnyValue), "]");//enables for expressions inside of array access
			
			if (i == nullptr)
			{
				//TODO complain
				return nullptr;
			}

			auto subA = new_sptr<SubArrayValue>();

			subA->array = v;
			subA->index = i;
			subA->last = tokens.peekBack();

			v = subA;

		}
		else
		{
			break;
		}

		if (v == nullptr)
		{
			//TODO complain
			break;
		}
		
	}

	while (tokens.hasNext())
	{
		auto const& tkn = tokens.current();

		if (tkn->type != TokenType::KEYWORD)
		{
			break;
		}

		tokens.consume();

		if (tkn->str == "is")
		{
			auto isa = new_sptr<IsAValue>();

			isa->val = v;
			isa->chkType = parseTypeName();

			v = isa;

		}
		else if (tkn->str == "as")
		{
			auto cast = new_sptr<CastValue>();

			cast->lhs = v;
			cast->castTarget = parseTypeName();

			v = cast;

		}
		else
		{
			tokens.rewind();
			break;
		}

	}

	return v;
}

sptr<Value> Parser::parseLiteral()
{
	auto const& first = tokens.current();

	if (first->type == TokenType::KEYWORD)
	{
		if (first->str == "this")
		{
			return new_sptr<VarReadValue>(first);
		}
		else if (first->str == "null")
		{
			return new_sptr<NullValue>(first);
		}

		return nullptr;
	}

	switch (first->type)
	{
		case TokenType::LITERAL_INT: return new_sptr<IntLiteralValue>(first);
		case TokenType::LITERAL_FLOAT: return new_sptr<FloatLiteralValue>(first);
		case TokenType::LITERAL_BOOL: return new_sptr<BoolLitValue>(first);
		case TokenType::LITERAL_STR: return new_sptr<StringLitValue>(first);
	}

	if (first->type == TokenType::START_BRACKET)
	{
		auto arrLit = new_sptr<ArrayLitValue>();

		arrLit->start = first;

		while (tokens.hasNext())
		{
			auto const& tkn = tokens.next();

			if (tkn->type == TokenType::COMMA)
			{
				continue;
			}

			if (tkn->type == TokenType::END_BRACKET)
			{
				arrLit->end = tkn;
				tokens.consume();
				break;
			}

			auto v = parseAnyValue();

			if (v == nullptr)
			{
				//TODO complain
				break;
			}

			arrLit->values.push_back(v);

		}

		return arrLit;
	}

	return nullptr;
}

sptr<Value> Parser::parseAnyAccess()
{
	return parseAccess(nullptr);
}

sptr<Value> Parser::parseAccess(sptr<Value> target)
{
	auto const& first = tokens.current();

	if (first->type != TokenType::IDENTIFIER)
	{
		return target;
	}

	auto access = new_sptr<VarChainValue>();

	access->target = target;
	
	while (true)
	{
		if (tokens.hasNext())
		{
			auto const& peek = tokens.peek();

			//check for method call
			if (peek->str == GENERIC_START || peek->type == TokenType::START_PAREN)
			{
				if (auto fnCall = parseFnCall(access))
				{
					return fnCall;
				}

			}

		}
		
		access->chain.push_back(tokens.current());

		if (!tokens.hasNext(2))
		{
			break;
		}

		if (tokens.next()->type != TokenType::PERIOD)
		{
			break;
		}

		if (tokens.next()->type != TokenType::IDENTIFIER)
		{
			//TODO complain
			break;
		}

	}

	return access;
}

sptr<Value> Parser::parseAnyExpr()
{
	return parseExpr(OP_PRECEDENCE_MAX);
}

sptr<Value> Parser::parseExpr(uint32_t precedence)
{
	auto lhs = parseNonExpr();

	while (tokens.hasNext())
	{
		auto const& opTkn = tokens.current();

		if (opTkn->type != TokenType::OPERATOR)
		{
			break;
		}

		auto found = INFIX_OPS.find(opTkn->str);
		if (found == INFIX_OPS.end())
		{
			break;
		}

		auto op = found->second;

		auto opWeight = OP_PRECEDENCE.at(op);

		//TODO this is dubious
		if (opWeight > precedence)
		{
			return lhs;
		}

		auto const& tkn = tokens.next();

		//Yes, we use *this* code to parse setters.
		if (tkn->str == "=")
		{
			tokens.consume();

			auto set = new_sptr<SetterValue>();

			set->lhs = lhs;
			set->op = op;
			set->rhs = parseAnyExpr();

			return set;
		}

		auto expr = new_sptr<ExpressionValue>();

		auto rhs = parseExpr(opWeight - 1);//TODO suppress errors here

		if (rhs == nullptr)
		{
			//TODO complain
		}

		expr->lValue = lhs;
		expr->op = op;
		expr->rValue = rhs;

		lhs = expr;

		break;
	}

	return lhs;
}

sptr<Value> Parser::parseAnyFnCall()
{
	return parseFnCall(nullptr);
}

sptr<Value> Parser::parseFnCall(sptr<Value> start)
{
	auto tkn = tokens.current();

	if (tkn->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	auto const& name = tkn;

	auto ret = new_sptr<FnCallValue>();

	tokens.consume();
	
	ret->genArgs = parseGenericArgs();

	if (ret->genArgs == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	tkn = tokens.current();

	if (tkn->type != TokenType::START_PAREN)
	{
		return nullptr;
	}

	tokens.consume();

	ret->name = name;
	ret->target = start;

	if (!parseValueList(ret->args, false))
	{
		//TODO complain
	}

	ret->end = tokens.current();

	return ret;
}

std::vector<sptr<Variable>> Parser::parseLocalVars()
{
	std::vector<sptr<Variable>> vars;
	auto pvars = parseLocalVarLike();

	for (auto const& pv : pvars)
	{
		vars.push_back(new_sptr<LocalVariable>(*pv));
	}

	return vars;
}

sptr<Variable> Parser::parseGlobalVar()
{
	auto first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (first->str != "const")
	{
		return nullptr;
	}

	auto tkn = tokens.next();

	sptr<ParsedType> type = nullptr;
	if (tkn->str == ":")
	{
		type = parseTypeName();
		tkn = tokens.next();
	}

	sptr<Token> name = tkn;

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	tkn = tokens.next();

	if (tkn->str != "=")
	{
		//TODO complain
		return nullptr;
	}

	tokens.consume();

	sptr<Value> initVal = parseAnyValue();

	if (!initVal->isCompileTimeConst())
	{
		//TODO complain
		return nullptr;
	}

	auto v = new_sptr<GlobalVariable>();

	v->first = first;
	v->nameTkn = name;
	v->typeHint = type;
	v->initValue = initVal;

	return v;
}

sptr<Variable> Parser::parseMemberVar()
{
	auto mods = parseStmtMods();

	bool isConst = false;
	auto start = tokens.current();

	if (start->type == TokenType::KEYWORD && start->str == "const")
	{
		isConst = true;
		tokens.consume();
	}

	auto type = parseTypeName();

	if (type == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	auto name = tokens.current();
	sptr<Value> initVal = nullptr;

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	tokens.consume();

	if (tokens.current()->str == "=")
	{
		tokens.consume();
		initVal = parseAnyValue();

	}

	auto v = new_sptr<MemberVariable>();

	v->mods = mods;
	v->first = start;
	v->nameTkn = name;
	v->typeHint = type;
	v->initValue = initVal;
	v->isConst = isConst;
	
	return v;
}

std::vector<sptr<ParsedVar>> Parser::parseLocalVarLike()
{
	std::vector<sptr<ParsedVar>> vars;
	auto first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		return vars;
	}

	bool isConst = false;

	if (first->str == "const")
	{
		isConst = true;
	}
	else if (first->str != "var")
	{
		return vars;
	}

	auto tkn = tokens.next();
	sptr<ParsedType> typeHint = nullptr;

	if (tkn->str == ":")
	{
		tkn = tokens.next();
		typeHint = parseTypeName();

		if (typeHint == nullptr)
		{
			//TODO complain
		}

	}

	auto varNames = parseIdentifierList();
	
	tkn = tokens.next();

	if (tkn->str == ":")
	{
		//postParseException(new_uptr<ParseException>("That's not how type hints work in Caliburn. Type hints go before the variable name, not after. So var: int x is valid. var x: int isn't.", tkn));
		return vars;
	}

	sptr<Value> initValue = nullptr;

	if (tkn->str == "=")
	{
		tokens.consume();
		initValue = parseAnyValue();

	}
	else if (typeHint == nullptr)
	{
		//postParseException(new_uptr<ParseException>("Implicitly-typed variable must be manually initialized", tkn));
		return vars;
	}

	for (auto const& name : varNames)
	{
		auto v = new_sptr<ParsedVar>();

		v->first = first;
		v->isConst = isConst;
		v->typeHint = typeHint;
		v->name = name;
		v->initValue = initValue;

		vars.push_back(v);

	}

	return vars;
}

std::vector<sptr<ParsedVar>> Parser::parseMemberVarLike()
{
	std::vector<sptr<ParsedVar>> vars;
	bool isConst = false;
	auto first = tokens.current();

	if (first->type == TokenType::KEYWORD && first->str == "const")
	{
		isConst = true;
		tokens.consume();
	}

	auto typeHint = parseTypeName();

	if (typeHint == nullptr)
	{
		return vars;
	}

	auto varNames = parseIdentifierList();

	if (varNames.empty())
	{
		//TODO complain
		return vars;
	}

	sptr<Value> initValue = nullptr;

	if (tokens.current()->str == "=")
	{
		initValue = parseAnyValue();

		if (initValue == nullptr)
		{
			//TODO complain
		}

	}

	for (auto& name : varNames)
	{
		auto v = new_sptr<ParsedVar>();

		v->first = first;
		v->isConst = isConst;
		v->typeHint = typeHint;
		v->name = name;
		v->initValue = initValue;

		vars.push_back(v);

	}

	return vars;
}

std::vector<uptr<ParsedFnArg>> Parser::parseFnArgs()
{
	std::vector<uptr<ParsedFnArg>> fnArgs;

	while (tokens.hasNext())
	{
		auto argType = parseTypeName();

		if (argType == nullptr)
		{
			break;
		}

		auto arg = new_uptr<ParsedFnArg>();

		arg->typeHint = argType;
		arg->name = tokens.current();

		if (arg->name->type != TokenType::IDENTIFIER)
		{
			errors->err("Not an argument name", arg->name, nullptr);
			break;
		}

		tokens.consume();

		fnArgs.push_back(std::move(arg));

		if (tokens.current()->type == TokenType::COMMA)
		{
			tokens.consume();
			continue;
		}

		break;
	}

	return fnArgs;
}

std::vector<uptr<Annotation>> Parser::parseAllAnnotations()
{
	std::vector<uptr<Annotation>> vec;

	while (tokens.hasNext())
	{
		auto a = parseAnnotation();

		if (a == nullptr)
		{
			break;
		}

		vec.push_back(std::move(a));

	}

	return vec;
}

uptr<Annotation> Parser::parseAnnotation()
{
	auto first = tokens.current();

	if (first->str != "@")
	{
		return nullptr;
	}

	auto name = tokens.peek();

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	tokens.consume(2);

	std::vector<sptr<Token>> content;
	
	if (tokens.current()->type == TokenType::START_PAREN)
	{
		tokens.consume();
		int closingParenNeeded = 1;

		while (tokens.hasNext())
		{
			auto tkn = tokens.next();

			if (tkn->type == TokenType::END_PAREN)
			{
				--closingParenNeeded;

				if (closingParenNeeded == 0)
				{
					tokens.consume();
					break;
				}

			}
			else
			{
				if (tkn->type == TokenType::START_PAREN)
				{
					++closingParenNeeded;
				}

				content.push_back(tkn);
			}

		}

		if (closingParenNeeded != 0)
		{
			//TODO complain
		}

	}

	auto a = new_uptr<Annotation>();

	a->first = first;
	a->name = name;
	a->last = tokens.current();
	a->contents = content;

	tokens.consume();
	
	return a;
}
