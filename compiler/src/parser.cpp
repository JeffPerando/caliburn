
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

bool Parser::parseAnyBetween(std::string start, std::function<void()> fn, std::string end)
{
	if (tokens.current()->str != start)
	{
		//TODO complain
		return false;
	}

	tokens.consume();

	fn();

	if (tokens.current()->str != end)
	{
		//TODO complain
		return false;
	}

	tokens.consume();

	return true;
}

std::vector<sptr<Token>> Parser::parseIdentifierList()
{
	std::vector<sptr<Token>> ids;

	while (true)
	{
		if (tokens.current()->type != TokenType::IDENTIFIER)
		{
			break;
		}

		ids.push_back(tokens.current());

		if (!tokens.hasNext(2))
		{
			break;
		}

		if (tokens.next()->type != TokenType::COMMA)
		{
			break;
		}

		tokens.consume();

	}

	return ids;
}

sptr<GenericSignature> Parser::parseGenericSig()
{
	sptr<Token> first = tokens.current();
	auto start = tokens.currentIndex();
	
	if (first->str != GENERIC_START)
	{
		//We return an empty signature since every generic needs one, even a blank one.
		return new_sptr<GenericSignature>();
	}

	std::vector<GenericName> names;

	bool parsingDefaults = false;

	while (tokens.hasNext(2))
	{
		sptr<Token> t = tokens.current();
		sptr<Token> n = tokens.peek();

		if (t->type != TokenType::KEYWORD)
		{
			break;
		}

		if (n->type != TokenType::IDENTIFIER)
		{
			//TODO complain
			break;
		}

		tokens.consume(2);

		GenericResult defRes;

		if (tokens.current()->str == "=")
		{
			parsingDefaults = true;

			tokens.consume();

			if (auto v = parseLiteral())
			{
				defRes = v;
			}
			else if (auto t = parseTypeName())
			{
				defRes = t;
			}
			else
			{
				//TODO complain
			}

		}
		else if (parsingDefaults)
		{
			//TODO complain
			//Generics defaults must always be at the end
			//I may reconsider this rule if explicit-named generics become a thing. But that logic is too hard right now
		}

		auto genName = GenericName(t, n, defRes);

		names.push_back(genName);

		if (tokens.current()->str != ",")
		{
			break;
		}

		tokens.consume();

	}

	sptr<Token> last = tokens.current();

	if (last->str != GENERIC_END)
	{
		tokens.revertTo(start);
		return nullptr;
	}

	tokens.consume();

	auto sig = new_sptr<GenericSignature>(names);

	sig->first = first;
	sig->last = last;

	return sig;
}

sptr<GenericArguments> Parser::parseGenericArgs()
{
	sptr<Token> first = tokens.current();
	auto start = tokens.currentIndex();
	
	if (first->str != GENERIC_START)
	{
		return new_sptr<GenericArguments>();
	}

	tokens.consume();

	std::vector<GenericResult> results;

	while (tokens.hasNext())
	{
		GenericResult result;

		if (auto v = parseLiteral())
		{
			result = v;
		}
		else if (auto t = parseTypeName())
		{
			result = t;
		}
		else break;

		results.push_back(result);

		if (tokens.current()->str != ",")
		{
			break;
		}

		tokens.consume();

	}

	auto& last = tokens.current();

	if (last->str != GENERIC_END)
	{
		tokens.revertTo(start);
		return nullptr;
	}

	tokens.consume();

	auto args = new_sptr<GenericArguments>(results);

	args->first = first;
	args->last = last;

	return args;
}

std::vector<sptr<Value>> Parser::parseValueList(bool commaOptional)
{
	std::vector<sptr<Value>> values;

	while (tokens.hasNext())
	{
		auto v = parseAnyValue();

		if (v == nullptr)
		{
			//TODO complain?
			break;
		}

		values.push_back(v);

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

	return values;
}

bool Parser::parseSemicolon()
{
	auto& end = tokens.current();

	if (end->type == TokenType::END)
	{
		tokens.consume();
		return true;
	}

	//postParseException(new_uptr<UnexpectedTokenException>(tkn, ';'));
	return false;
}

bool Parser::parseScopeEnd(ref<uptr<ScopeStatement>> stmt)
{
	sptr<Token> first = tokens.current();
	auto tknIndex = tokens.currentIndex();
	
	if (first->type != TokenType::KEYWORD)
	{
		return false;
	}

	tokens.consume();

	auto retMode = RETURN_MODES.find(first->str);

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
		auto& mod = tokens.current();

		if (mod->str == "public")
			mods.PUBLIC = 1;
		else if (mod->str == "private")
			mods.PRIVATE = 1;
		else if (mod->str == "protected")
			mods.PROTECTED = 1;
		else if (mod->str == "shared")
			mods.SHARED = 1;
		else if (mod->str == "static")
			mods.STATIC = 1;
		else break;

		tokens.consume();

	}

	return mods;
}

uptr<ScopeStatement> Parser::parseScope(std::vector<ParseMethod<uptr<Statement>>> pms)
{
	auto mods = parseStmtMods();

	sptr<Token> first = tokens.current();

	if (first->type != TokenType::START_SCOPE)
	{
		return nullptr;
	}

	tokens.consume();

	auto scope = new_uptr<ScopeStatement>();

	scope->first = first;

	while (tokens.hasNext())
	{
		if (tokens.current()->type == TokenType::END_SCOPE)
		{
			scope->last = tokens.current();

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

	while (tokens.hasNext(2) && tokens.current()->type == TokenType::START_BRACKET)
	{
		tokens.consume();

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
		type->lastToken = tokens.current();

		tokens.consume();

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
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD || first->str != "import")
	{
		return nullptr;
	}

	auto& modName = tokens.next();

	if (modName->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	tokens.consume();

	auto ret = new_uptr<ImportStatement>(first);

	ret->name = modName;

	if (tokens.current()->str == "as")
	{
		if (tokens.peek()->type != TokenType::IDENTIFIER)
		{
			//TODO complain
		}

		ret->alias = tokens.next();

		tokens.consume();

	}

	return ret;
}

uptr<Statement> Parser::parseModuleDef()
{
	auto& start = tokens.current();

	if (start->type != TokenType::KEYWORD || start->str != "module")
	{
		return nullptr;
	}

	auto& name = tokens.next();

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	return new_uptr<ModuleStatement>(start, name);
}

uptr<Statement> Parser::parseTypedef()
{
	//type x = y;
	if (!tokens.hasNext(5))
	{
		return nullptr;
	}

	auto& start = tokens.current();

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

	auto& name = tokens.next();

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

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
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD && first->str != "shader")
	{
		return nullptr;
	}

	auto& name = tokens.next();

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	auto shader = new_uptr<ShaderStatement>();

	shader->first = first;
	shader->name = name;

	tokens.consume();

	if (tokens.current()->type == TokenType::START_PAREN)
	{
		tokens.consume();

		while (tokens.hasNext())
		{
			if (tokens.current()->type == TokenType::END_PAREN)
			{
				tokens.consume();
				break;
			}

			auto type = parseTypeName();
			
			if (type == nullptr)
			{
				//TODO complain
				break;
			}

			auto& name = tokens.current();

			if (name->type != TokenType::IDENTIFIER)
			{
				//TODO complain
				break;
			}

			tokens.consume();

			shader->descriptors.push_back(std::pair(type, name));

			if (tokens.current()->type == TokenType::COMMA)
			{
				tokens.consume();
				continue;
			}
			
		}

		if (shader->descriptors.empty())
		{
			//TODO complain?
		}

	}

	if (tokens.current()->type != TokenType::START_SCOPE)
	{
		//TODO complain
		return shader;
	}

	tokens.consume();

	while (true)
	{
		if (tokens.current()->type == TokenType::END_SCOPE)
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
		
		if (!parseSemicolon())
		{
			//TODO complain
		}

	}

	return shader;
}

uptr<Statement> Parser::parseStruct()
{
	sptr<Token> first = tokens.current();

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

	auto& name = tokens.next();

	if (name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
	}

	auto stmt = new_uptr<StructStatement>(name, stmtType);

	stmt->first = first;

	tokens.consume();

	if (tokens.current()->type != TokenType::START_SCOPE)
	{
		//TODO complain
	}

	tokens.consume();

	while (true)
	{
		auto& end = tokens.current();

		if (end->type == TokenType::END_SCOPE)
		{
			stmt->last = end;
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
		else
		{
			//TODO complain
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
	sptr<Token> first = tokens.current();

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

	if (tokens.current()->type == TokenType::ARROW)
	{
		tokens.consume();
		fn->retType = parseTypeName();

	}
	else
	{
		fn->retType = new_sptr<ParsedType>("void");
	}

	if (fn->retType == nullptr)
	{
		//TODO complain
	}

	auto& scopeStart = tokens.current();
	
	if (scopeStart->type != TokenType::START_SCOPE)
	{
		auto e = errors->err("Invalid start to function scope", scopeStart, nullptr);

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

	if (tokens.current()->type != TokenType::KEYWORD)
	{
		//TODO complain
		//tokens.revertTo(start);
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
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD || first->str != "if")
	{
		return nullptr;
	}

	tokens.consume();

	auto stmt = new_uptr<IfStatement>();

	stmt->first = first;

	parseAnyBetween("(", lambda() {
		stmt->condition = parseAnyValue();
	}, ")");

	stmt->innerIf = parseScope({&Parser::parseDecl});

	if (tokens.current()->str == "else")
	{
		tokens.consume();

		stmt->innerIf = parseScope({ &Parser::parseDecl });

	}

	return stmt;
}

uptr<Statement> Parser::parseFor() //FIXME
{
	return nullptr;
}

uptr<Statement> Parser::parseWhile()
{
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD || first->str != "while")
	{
		return nullptr;
	}

	auto stmt = new_uptr<WhileStatement>();
	
	stmt->first = first;

	parseAnyBetween("(", lambda() {
		stmt->condition = parseAnyValue();
	}, ")");

	stmt->loop = parseScope({ &Parser::parseLogic });

	return stmt;
}

uptr<Statement> Parser::parseDoWhile()
{
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD || first->str != "do")
	{
		return nullptr;
	}

	tokens.consume();

	auto ret = new_uptr<WhileStatement>();

	ret->first = first;
	ret->doWhile = true;
	ret->loop = parseScope({&Parser::parseLogic});

	if (tokens.current()->type != TokenType::KEYWORD || tokens.current()->str != "while")
	{
		//TODO complain
		return nullptr;
	}

	parseAnyBetween("(", lambda() {
		ret->condition = parseAnyValue();
	}, ")");
	
	return ret;
}

uptr<Statement> Parser::parseValueStmt()
{
	std::vector<ParseMethod<sptr<Value>>> methods = {
		&Parser::parseAnyFnCall,
		&Parser::parseAnyExpr
	};

	if (auto val = parseAny(methods))
	{
		return new_uptr<ValueStatement>(val);
	}

	return nullptr;
}

uptr<Statement> Parser::parseGlobalVarStmt()
{
	sptr<Token> first = tokens.current();

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
	if (tokens.current()->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	auto stmt = new_uptr<VariableStatement>();

	stmt->first = tokens.current();
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
		{ &Parser::parseParenValue, &Parser::parseUnaryValue, &Parser::parseLiteral, &Parser::parseAnyFnCall, &Parser::parseAnyAccess };

	sptr<Value> v = parseAny(initParsers);

	if (v == nullptr)
	{
		return nullptr;
	}

	while (tokens.hasNext())
	{
		auto& tkn = tokens.current();

		if (tkn->type == TokenType::PERIOD && tokens.peek()->type == TokenType::IDENTIFIER)
		{
			tokens.consume();
			v = parseAccess(v);
		}
		else if (tkn->type == TokenType::START_BRACKET)
		{
			auto subA = new_sptr<SubArrayValue>();

			subA->array = v;

			parseAnyBetween("[", lambda() {
				if (auto i = parseAnyValue())
				{
					subA->index = i;
				}
				else
				{
					//TODO complain
				}
			}, "]");
			
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
		auto& kywd = tokens.current();

		if (kywd->type != TokenType::KEYWORD)
		{
			break;
		}

		tokens.consume();

		if (kywd->str == "is")
		{
			auto isa = new_sptr<IsAValue>();

			isa->val = v;
			isa->chkType = parseTypeName();

			v = isa;

		}
		else if (kywd->str == "as")
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
	sptr<Token> first = tokens.current();

	tokens.consume();

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

		tokens.rewind();
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
		arrLit->values = parseValueList(true);

		if (arrLit->values.empty())
		{
			//TODO complain
		}

		sptr<Token> last = tokens.current();

		if (last->type != TokenType::END_BRACKET)
		{
			//TODO complain
		}

		arrLit->end = last;

		tokens.consume();

		return arrLit;
	}

	tokens.rewind();
	return nullptr;
}

sptr<Value> Parser::parseUnaryValue()
{
	sptr<Token> first = tokens.current();

	if (first->type == TokenType::KEYWORD)
	{
		tokens.consume();

		if (first->str == "sign")
		{
			return new_sptr<SignValue>(first, parseNonExpr());
		}
		else if (first->str == "unsign")
		{
			return new_sptr<UnsignValue>(first, parseNonExpr());
		}

		tokens.rewind();
		return nullptr;
	}
	else if (first->type == TokenType::OPERATOR)
	{
		auto foundOp = UNARY_OPS.find(first->str);

		if (foundOp != UNARY_OPS.end())
		{
			tokens.consume();

			auto unary = new_sptr<UnaryValue>();

			unary->start = first;
			unary->op = foundOp->second;
			unary->val = parseAnyExpr();

			if (foundOp->second == Operator::ABS)
			{
				if (tokens.current()->str != "|")
				{
					//TODO complain
					return nullptr;
				}

				unary->end = tokens.current();

				tokens.consume();

			}

			return unary;
		}

	}

	return nullptr;
}

sptr<Value> Parser::parseParenValue()
{
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::START_PAREN)
	{
		return nullptr;
	}

	tokens.consume();

	auto v = parseAnyValue();

	//TODO tuple literals go here

	sptr<Token> last = tokens.current();

	if (last->type != TokenType::END_PAREN)
	{
		//TODO complain
	}

	tokens.consume();

	return v;
}

sptr<Value> Parser::parseAnyAccess()
{
	return parseAccess(nullptr);
}

sptr<Value> Parser::parseAccess(sptr<Value> target)
{
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::IDENTIFIER)
	{
		return target;
	}

	auto access = new_sptr<VarChainValue>();

	access->target = target;
	
	while (true)
	{
		if (tokens.hasNext(3))
		{
			auto& fwd = tokens.peek();

			//check for method call
			if (fwd->str == GENERIC_START || fwd->type == TokenType::START_PAREN)
			{
				if (auto fnCall = parseFnCall(access->chain.empty() ? target : access))
				{
					return fnCall;
				}

			}

		}
		
		access->chain.push_back(tokens.current());

		tokens.consume();

		if (!tokens.hasNext(2))
		{
			break;
		}

		if (tokens.peek(1)->type != TokenType::PERIOD)
		{
			break;
		}

		if (tokens.peek(2)->type != TokenType::IDENTIFIER)
		{
			break;
		}

		tokens.consume(2);

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

	while (tokens.hasNext(2))
	{
		auto& opTkn = tokens.current();

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

		if (opWeight > precedence)
		{
			return lhs;
		}

		tokens.consume();

		//Yes, we use *this* code to parse setters.
		if (tokens.current()->str == "=")
		{
			tokens.consume();

			auto set = new_sptr<SetterValue>();

			set->lhs = lhs;
			set->op = op;
			set->rhs = parseAnyExpr();

			return set;
		}

		auto rhs = parseExpr(opWeight - 1);//TODO suppress errors here

		if (rhs == nullptr)
		{
			//TODO complain
		}

		auto expr = new_sptr<ExpressionValue>();

		expr->lValue = lhs;
		expr->op = op;
		expr->rValue = rhs;

		lhs = expr;

	}

	return lhs;
}

sptr<Value> Parser::parseAnyFnCall()
{
	return parseFnCall(nullptr);
}

sptr<Value> Parser::parseFnCall(sptr<Value> start)
{
	sptr<Token> name = tokens.current();

	if (name->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	tokens.consume();

	auto fnCall = new_sptr<FnCallValue>();

	fnCall->name = name;
	fnCall->target = start;
	fnCall->genArgs = parseGenericArgs();

	if (fnCall->genArgs == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	if (!parseAnyBetween("(", lambda() {
		fnCall->args = parseValueList(false);
	}, ")"))
	{
		//TODO complain
		return nullptr;
	}

	fnCall->end = tokens.peekBack();

	return fnCall;
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
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (first->str != "const")
	{
		return nullptr;
	}

	auto v = new_sptr<GlobalVariable>();

	v->first = first;

	if (tokens.current()->str == ":")
	{
		v->typeHint = parseTypeName();
	}
	
	if (tokens.current()->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	v->nameTkn = tokens.current();

	tokens.consume();

	if (tokens.current()->str != "=")
	{
		//TODO complain
		return nullptr;
	}

	tokens.consume();

	v->initValue = parseAnyValue();

	if (!v->initValue->isCompileTimeConst())
	{
		//TODO complain
		return nullptr;
	}

	return v;
}

sptr<Variable> Parser::parseMemberVar()
{
	auto mods = parseStmtMods();

	bool isConst = false;
	auto& start = tokens.current();

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

	auto& name = tokens.current();
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
	sptr<Token> first = tokens.current();

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

	tokens.consume();

	sptr<ParsedType> typeHint = nullptr;

	if (tokens.current()->str == ":")
	{
		tokens.consume();
		typeHint = parseTypeName();

		if (typeHint == nullptr)
		{
			//TODO complain
		}

	}

	auto varNames = parseIdentifierList();
	
	tokens.consume();

	if (tokens.current()->str == ":")
	{
		//postParseException(new_uptr<ParseException>("That's not how type hints work in Caliburn. Type hints go before the variable name, not after. So var: int x is valid. var x: int isn't.", tkn));
		return vars;
	}

	sptr<Value> initValue = nullptr;

	if (tokens.current()->str == "=")
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
	sptr<Token> first = tokens.current();

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
	if (!tokens.hasNext(2))
	{
		return nullptr;
	}

	sptr<Token> first = tokens.current();

	if (first->str != "@")
	{
		return nullptr;
	}

	auto a = new_uptr<Annotation>();

	a->first = first;
	a->name = tokens.next();

	if (a->name->type != TokenType::IDENTIFIER)
	{
		//TODO complain
		return nullptr;
	}

	tokens.consume();

	if (tokens.current()->type == TokenType::START_PAREN)
	{
		tokens.consume();
		int closingParenNeeded = 1;

		while (tokens.hasNext())
		{
			auto& tkn = tokens.current();

			if (tkn->type == TokenType::END_PAREN)
			{
				--closingParenNeeded;

				if (closingParenNeeded == 0)
				{
					tokens.consume();
					break;
				}

			}
			else if (tkn->type == TokenType::START_PAREN)
			{
				++closingParenNeeded;
			}

			a->contents.push_back(tkn);
			tokens.consume();

		}

		if (closingParenNeeded != 0)
		{
			//TODO complain
		}

		a->last = tokens.current();

		tokens.consume();

	}
	else
	{
		a->last = a->name;
	}

	return a;
}
