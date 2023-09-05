
#include "parser.h"

#include <stack>

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

void Parser::skipStmt()
{
	size_t scopes = 0;

	while (tokens.hasNext())
	{
		sptr<Token> tkn = tokens.current();

		if (tkn->type == TokenType::START_SCOPE)
		{
			++scopes;
		}
		else if (tkn->type == TokenType::END_SCOPE)
		{
			if (scopes > 0)
			{
				--scopes;
			}
		}
		
		tokens.consume();

		if (scopes == 0 && tkn->type == TokenType::END)
		{
			break;
		}

	}

}

std::vector<uptr<Statement>> Parser::parse()
{
	std::vector<uptr<Statement>> ast;

	while (tokens.hasNext())
	{
		sptr<Token> start = tokens.current();

		if (auto finished = parseDecl())
		{
			ast.push_back(std::move(finished));
		}
		else
		{
			errors->err("Invalid start to a declaration", start);
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
	size_t current = tokens.currentIndex();

	for (auto& fn : fns)
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
		return nullptr;
	}

	tokens.consume();

	auto ret = fn(*this);

	if (tokens.current()->str != end)
	{
		auto e = errors->err({ "Unexpected token found; Expected", end, "got", tokens.current()->str });
		return ret;
	}

	tokens.consume();

	return ret;
}

bool Parser::parseAnyBetween(std::string start, std::function<void()> fn, std::string end)
{
	sptr<Token> startTkn = tokens.current();

	if (startTkn->str != start)
	{
		auto e = errors->err({ "Unexpected token found; Expected", start }, startTkn);

		return false;
	}

	tokens.consume();

	fn();

	sptr<Token> endTkn = tokens.current();

	if (endTkn->str != end)
	{
		auto e = errors->err({ "Unexpected token found; Expected", end}, startTkn, endTkn);
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
			auto e = errors->err("Names within generic signatures must be a valid identifier", n);
			break;
		}

		tokens.consume(2);

		GenericResult defRes;

		if (tokens.current()->str == "=")
		{
			tokens.consume();

			parsingDefaults = true;

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
				auto e = errors->err("Invalid generic default starts here", tokens.current());
			}

		}
		else if (parsingDefaults)
		{
			auto e = errors->err("Generic signature defaults must always be at the end", tokens.current());
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

void Parser::parseSemicolon()
{
	auto& end = tokens.current();

	if (end->type == TokenType::END)
	{
		tokens.consume();
	}
	else
	{
		auto e = errors->err({ "Expected a semicolon, got", end->str }, end);
	}

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
		auto e = errors->err("Not a valid statement or end of scope", first);

		tokens.revertTo(tknIndex);

		return false;
	}

	stmt->retMode = retMode->second;

	if (stmt->retMode == ReturnMode::RETURN)
	{
		stmt->retValue = parseAnyValue();
	}
	
	parseSemicolon();

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

uptr<ScopeStatement> Parser::parseScope(std::vector<ParseMethod<uptr<Statement>>> pms, bool err)
{
	auto mods = parseStmtMods();

	sptr<Token> first = tokens.current();

	if (first->type != TokenType::START_SCOPE)
	{
		if (err)
		{
			auto e = errors->err("Expected a { here", first);

			e->suggest("Wrap your code in a scope");

		}
		
		return nullptr;
	}

	tokens.consume();

	auto scope = new_uptr<ScopeStatement>();

	scope->first = first;

	while (tokens.hasNext())
	{
		if (parseScopeEnd(scope))
		{
			if (tokens.current()->type != TokenType::END_SCOPE)
			{
				auto e = errors->err("Found an end-scope statement, but it did not end the scope", tokens.current());
				continue;
			}

			scope->last = tokens.current();

			tokens.consume();
			break;
		}

		if (tokens.current()->type == TokenType::END_SCOPE)
		{
			tokens.consume();
			break;
		}

		for (auto& pm : pms)
		{
			if (auto stmt = pm(*this))
			{
				parseSemicolon();

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
				auto e = errors->err("Array length must be a compile-time constant", *len);
			}

			gArgs->args.push_back(len);
		}

		if (tokens.current()->type != TokenType::END_BRACKET)
		{
			auto e = errors->err("Array types must end with a ]", tokens.current());

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

	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		auto e = errors->err("Invalid start to declaration", first);
		return nullptr;
	}

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
		auto e = errors->err("Invalid start to declaration", first);
		return stmt;
	}

	for (auto& a : annotations)
	{
		stmt->annotations.emplace(a->name->str, std::move(a));
	}

	stmt->mods = mods;
	
	parseSemicolon();

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
		auto e = errors->err("Not a valid module name", modName);

		if (modName->type == TokenType::KEYWORD)
		{
			e->suggest("Can't import reserved keywords; try renaming your module");
		}

		if (modName->str == "*")
		{
			e->suggest("Can't import everything; try an actual module name");
		}

		return nullptr;
	}

	tokens.consume();

	auto ret = new_uptr<ImportStatement>(first);

	ret->name = modName;

	if (tokens.current()->str == "as")
	{
		sptr<Token> alias = tokens.next();

		if (alias->type == TokenType::IDENTIFIER)
		{
			ret->alias = tokens.next();

			tokens.consume();

		}
		else
		{
			auto e = errors->err("Invalid import alias", alias);

			if (alias->str == "*")
			{
				e->suggest("Wildcard imports are currently not supported");
			}

		}

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
		auto e = errors->err("Invalid module name", name);
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
		auto e = errors->err("Invalid type alias name", name);

		return nullptr;
	}

	if (tokens.next()->str != "=")
	{
		auto e = errors->err("Type definitions must have an equal sign to denote the aliased type", tokens.current());

		return nullptr;
	}

	sptr<Token> typeStart = tokens.next();

	if (sptr<ParsedType> aliasedType = parseTypeName())
	{
		return new_uptr<TypedefStatement>(start, name, aliasedType);
	}
	else
	{
		auto e = errors->err("Invalid start of type name", typeStart);

		if (typeStart->str == "[")
		{
			e->suggest("Array types are in the C-style format of x[], where x is the element type.");
		}

		if (typeStart->type == TokenType::KEYWORD)
		{
			e->suggest("Only keyword that's a valid type is the dynamic keyword");
		}

		return nullptr;
	}

}

uptr<Statement> Parser::parseShader()
{
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD && first->str != "shader")
	{
		return nullptr;
	}

	sptr<Token> name = tokens.next();

	if (name->type == TokenType::IDENTIFIER)
	{
		tokens.consume();
	}
	else
	{
		auto e = errors->err("Invalid shader object name", name);

		if (name->type == TokenType::START_SCOPE)
		{
			e->suggest("Name your shader so it can be found and compiled. Caliburn does not support 'anonymous' shaders");
		}

	}

	auto shader = new_uptr<ShaderStatement>();

	shader->first = first;
	shader->name = name;

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
				auto e = errors->err("Invalid descriptor type", tokens.current());
				break;
			}

			sptr<Token> name = tokens.current();

			if (name->type != TokenType::IDENTIFIER)
			{
				auto e = errors->err("Invalid descriptor name", tokens.current());
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

	}

	if (tokens.current()->type != TokenType::START_SCOPE)
	{
		auto e = errors->err("Shaders must start with a scope", tokens.current());
		skipStmt();
		return shader;
	}

	tokens.consume();

	while (tokens.hasNext())
	{
		sptr<Token> memStart = tokens.current();

		if (memStart->type != TokenType::KEYWORD)
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
				auto e = errors->err("Invalid shader object member; skipping statement", tokens.current());
				
				skipStmt();

				continue;
			}

			for (auto const& ioData : ios)
			{
				shader->ioVars.push_back(new_sptr<ShaderIOVariable>(*ioData));

			}

		}

		parseSemicolon();

	}

	while (tokens.current()->type != TokenType::END_SCOPE)
	{
		auto e = errors->err("Stray token; skipping", tokens.current());
		tokens.consume();
	}

	tokens.consume();

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

	sptr<Token> name = tokens.next();

	if (name->type == TokenType::IDENTIFIER)
	{
		tokens.consume();
	}
	else
	{
		auto e = errors->err("Invalid type name", name);

		if (name->type == TokenType::START_SCOPE)
		{
			e->suggest("Name your data structure");
		}

	}

	auto stmt = new_uptr<StructStatement>(name, stmtType);

	stmt->first = first;

	if (tokens.current()->type == TokenType::START_SCOPE)
	{
		tokens.consume();
	}
	else
	{
		auto e = errors->err("Types must start with a scope", tokens.current());
		skipStmt();
		return stmt;
	}

	while (true)
	{
		sptr<Token> end = tokens.current();

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
			stmt->members.emplace(fn->name->str, fn);

		}
		else
		{
			auto e = errors->err("Invalid start to type member", end);

		}
		
		parseSemicolon();

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
		sptr<Token> gpuDim = tokens.current();

		if (gpuDim->str == "[")
		{
			parseAnyBetween("[", lambda(){
				fn->invokeDims = parseIdentifierList();
			}, "]");

			if (fn->invokeDims.size() == 0)
			{
				auto e = errors->err("Invocation dimensions are empty", gpuDim, tokens.current());

				e->suggest("To use this feature, add identifiers separated with commas. Each identifier is a dimension within the GPU invocation (first is X, second is Y, etc.)");

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
		sptr<Token> genSigStart = tokens.current();

		fn->genSig = parseGenericSig();

		if (fn->genSig == nullptr)
		{
			auto e = errors->err("Invalid start to generic signature", genSigStart);
			return nullptr;
		}

	}
	
	parseAnyBetween("(", lambda(){
		fn->args = parseFnArgs();
	}, ")");

	sptr<Token> arrow = tokens.current();

	if (arrow->type == TokenType::ARROW)
	{
		sptr<Token> typeNameStart = tokens.next();

		if (auto type = parseTypeName())
		{
			fn->retType = type;
		}
		else
		{
			auto e = errors->err("Invalid return type after", arrow);

			if (typeNameStart->str == "void")
			{
				e->suggest("Void is not a proper type name; leave out the return type altogether");
			}

			if (typeNameStart->str == "dynamic")
			{
				e->suggest("Make a typedef statement; you can't make a dynamic type a return type directly since it has no name to look up");
			}

		}

	}
	else
	{
		fn->retType = new_sptr<ParsedType>("void");
	}

	sptr<Token> scopeStart = tokens.current();
	
	if (scopeStart->type == TokenType::START_SCOPE)
	{
		fn->code = parseScope({ &Parser::parseLogic });
	}
	else
	{
		auto e = errors->err("Invalid start to function scope", scopeStart);
	}

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
		sptr<Token> scopeStart = tokens.current();

		stmt->innerElse = parseScope({ &Parser::parseDecl });

		if (stmt->innerElse == nullptr)
		{

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

	sptr<Token> whileKwd = tokens.current();

	if (whileKwd->type != TokenType::KEYWORD || whileKwd->str != "while")
	{
		auto e = errors->err("This is a do/while loop, where is your while keyword?", whileKwd);
		return nullptr;
	}

	tokens.consume();

	parseAnyBetween("(", lambda() {
		ret->condition = parseAnyValue();
	}, ")");

	return ret;
}

uptr<Statement> Parser::parseValueStmt()
{
	std::vector<ParseMethod<sptr<Value>>> methods = {
		&Parser::parseAnyFnCall,
		&Parser::parseExpr
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
	sptr<Token> first = tokens.current();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (first->str != "const" && first->str != "var")
	{
		return nullptr;
	}

	auto stmt = new_uptr<VariableStatement>();

	stmt->first = tokens.current();
	stmt->vars = parseLocalVars();

	return stmt;
}

sptr<Value> Parser::parseAnyValue()
{
	std::vector<ParseMethod<sptr<Value>>> methods = {
		&Parser::parseExpr,//this will call the other two and look for an expression
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
		sptr<Token> tkn = tokens.current();

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
					auto e = errors->err("Invalid array index", tokens.current());
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
			auto e = errors->err("Parsing value failed here; see other errors", tkn);
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

	switch (first->type)
	{
		case TokenType::LITERAL_INT: return new_sptr<IntLiteralValue>(first);
		case TokenType::LITERAL_FLOAT: return new_sptr<FloatLiteralValue>(first);
		case TokenType::LITERAL_BOOL: return new_sptr<BoolLitValue>(first);
		case TokenType::LITERAL_STR: return new_sptr<StringLitValue>(first);
	}

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

	if (first->type == TokenType::START_BRACKET)
	{
		auto arrLit = new_sptr<ArrayLitValue>();

		arrLit->start = first;
		
		parseAnyBetween("[", lambda() {
			auto xs = parseValueList(true);

			if (xs.empty())
			{
				auto e = errors->err("Empty array literal", first, tokens.peek());

			}

			arrLit->values = xs;

		}, "]");
		
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
			unary->val = parseExpr();

			if (foundOp->second == Operator::ABS)
			{
				sptr<Token> absEnd = tokens.current();

				if (absEnd->str != "|")
				{
					auto e = errors->err("Absolute operator values must end with |", absEnd);

					return nullptr;
				}

				unary->end = absEnd;

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
		auto e = errors->err("Expected a closing parentheses", last);
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

sptr<Value> Parser::parseExpr()
{
	auto start = parseNonExpr();

	if (start == nullptr)
	{
		return nullptr;
	}

	std::stack<sptr<Value>> values;
	std::stack<Operator> ops;

	values.push(start);
	
	const auto makeExpr = lambda()
	{
		auto popOp = ops.top();
		ops.pop();

		auto rhs = values.top();
		values.pop();

		auto lhs = values.top();
		values.pop();

		auto expr = new_sptr<ExpressionValue>();

		expr->lValue = lhs;
		expr->op = popOp;
		expr->rValue = rhs;

		values.push(expr);

	};

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

		tokens.consume();

		auto op = found->second;
		auto opWeightCur = OP_PRECEDENCE.at(op);
		
		if (!ops.empty() && OP_PRECEDENCE.at(ops.top()) > opWeightCur)
		{
			makeExpr();
		}

		ops.push(op);

		auto val = parseNonExpr();

		if (val == nullptr)
		{
			auto e = errors->err("Expected a value here", tokens.current());
			break;
		}

		values.push(val);

	}
	
	while (values.size() > 1)
	{
		makeExpr();
	}

	return values.top();
}

sptr<Value> Parser::parseAnyFnCall()
{
	return parseFnCall(nullptr);
}

sptr<Value> Parser::parseFnCall(sptr<Value> start)
{
	if (tokens.hasNext(3))
	{
		return nullptr;
	}

	sptr<Token> name = tokens.current();

	if (name->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	tokens.consume();

	auto fnCall = new_sptr<FnCallValue>();

	fnCall->name = name;
	fnCall->target = start;

	sptr<Token> genArgStart = tokens.current();

	fnCall->genArgs = parseGenericArgs();

	if (fnCall->genArgs == nullptr)
	{
		auto e = errors->err("Invalid generic argument start", genArgStart);

		return nullptr;
	}

	if (!parseAnyBetween("(", lambda() {
		fnCall->args = parseValueList(false);
	}, ")"))
	{
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

	sptr<Token> name = tokens.current();
	
	if (name->type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid global variable name", name);
		return nullptr;
	}

	v->nameTkn = name;

	sptr<Token> eqSign = tokens.next();

	if (eqSign->str != "=")
	{
		auto e = errors->err("Global variables must be initialized", eqSign);

		e->suggest("Add an equal sign and a value");

		return v;
	}

	tokens.consume();

	if (auto init = parseAnyValue())
	{
		if (!init->isCompileTimeConst())
		{
			auto e = errors->err("Global variables initialized with a non-constant value", *init);

			e->suggest("Use a compile-time constant, like a literal. Struct-likes do not count.");

		}

		v->initValue = init;

	}
	else
	{
		auto e = errors->err("Could not find an initial value here", tokens.current());
	}

	return v;
}

sptr<Variable> Parser::parseMemberVar()
{
	auto mods = parseStmtMods();

	sptr<Token> start = tokens.current();
	bool isConst = false;
	
	if (start->type == TokenType::KEYWORD && start->str == "const")
	{
		isConst = true;
		tokens.consume();
	}

	sptr<Token> typeStart = tokens.current();
	auto type = parseTypeName();

	if (type == nullptr)
	{
		auto e = errors->err("Invalid member variable type name starts here", typeStart);

		return nullptr;
	}

	sptr<Token> name = tokens.current();
	
	if (name->type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid member variable name", name);

		return nullptr;
	}

	tokens.consume();

	auto v = new_sptr<MemberVariable>();

	v->mods = mods;
	v->first = start;
	v->nameTkn = name;
	v->typeHint = type;
	v->isConst = isConst;

	if (tokens.current()->str == "=")
	{
		tokens.consume();

		v->initValue = parseAnyValue();
	}

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

		if (auto t = parseTypeName())
		{
			typeHint = t;
		}
		else
		{
			sptr<Token> typeName = tokens.current();
			auto e = errors->err("Invalid type name", typeName);
		}

	}

	auto varNames = parseIdentifierList();

	if (varNames.empty())
	{
		sptr<Token> varName = tokens.current();
		auto e = errors->err("Invalid local variable name", varName);
	}

	tokens.consume();

	if (tokens.current()->str == ":")
	{
		auto e = errors->err("That's not how type hints work in Caliburn. Type hints go before the variable name, not after. So var: int x is valid. var x: int isn't.", tokens.current());

		return vars;
	}

	sptr<Value> initValue = nullptr;

	if (tokens.current()->str == "=")
	{
		sptr<Token> valStart = tokens.next();

		if (auto v = parseAnyValue())
		{
			initValue = v;
		}
		else
		{
			auto e = errors->err("Invalid initializer value", valStart);
		}

	}
	else if (typeHint == nullptr)
	{
		auto e = errors->err("Implicitly-typed variables must be manually initialized", first, tokens.current());
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
		auto e = errors->err("Invalid member variable name", tokens.current());
		return vars;
	}

	sptr<Value> initValue = nullptr;

	if (tokens.current()->str == "=")
	{
		if (auto v = parseAnyValue())
		{
			initValue = v;

		}
		else
		{
			auto e = errors->err("Invalid member variable initial value", tokens.current());
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
			errors->err("Not a valid argument name", arg->name);
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
	sptr<Token> name = tokens.peek();

	if (first->str != "@")
	{
		return nullptr;
	}

	if (name->type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Annotations must be named with an identifier", first, name);
		return nullptr;
	}

	tokens.consume(2);

	auto a = new_uptr<Annotation>();

	a->first = first;
	a->name = name;

	sptr<Token> startParen = tokens.current();

	if (startParen->type == TokenType::START_PAREN)
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
			auto e = errors->err("Ran out of symbols and could not close annotation", startParen);
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
