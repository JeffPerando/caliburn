
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

	while (tkns.hasCur())
	{
		sptr<Token> tkn = tkns.cur();

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
		
		tkns.consume();

		if (scopes == 0 && tkn->type == TokenType::END)
		{
			break;
		}

	}

}

std::vector<uptr<Statement>> Parser::parse()
{
	std::vector<uptr<Statement>> ast;

	while (tkns.hasCur())
	{
		sptr<Token> start = tkns.cur();

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
T Parser::parseAny(in<std::vector<ParseMethod<T>>> fns)
{
	size_t current = tkns.index();

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
			tkns.revertTo(current);

		}

	}

	return nullptr;
}

bool Parser::parseAnyBetween(in<std::string> start, in<std::function<void()>> fn, in<std::string> end)
{
	sptr<Token> startTkn = tkns.cur();

	if (startTkn->str != start)
	{
		auto e = errors->err({ "Unexpected token found; Expected", start }, startTkn);

		return false;
	}

	tkns.consume();

	fn();

	sptr<Token> endTkn = tkns.cur();

	if (endTkn->str != end)
	{
		auto e = errors->err({ "Unexpected token found; Expected", end}, startTkn, endTkn);
		return false;
	}

	tkns.consume();

	return true;
}

std::vector<sptr<Token>> Parser::parseIdentifierList()
{
	std::vector<sptr<Token>> ids;

	while (true)
	{
		if (tkns.cur()->type != TokenType::IDENTIFIER)
		{
			break;
		}

		ids.push_back(tkns.cur());

		if (!tkns.hasRem(2))
		{
			break;
		}

		if (tkns.next()->type != TokenType::COMMA)
		{
			break;
		}

		tkns.consume();

	}

	return ids;
}

uptr<GenericSignature> Parser::parseGenericSig()
{
	sptr<Token> first = tkns.cur();
	auto start = tkns.index();
	
	if (first->str != GENERIC_START)
	{
		//We return an empty signature since every generic needs one, even a blank one.
		return new_uptr<GenericSignature>();
	}

	std::vector<GenericName> names;

	bool parsingDefaults = false;

	while (tkns.hasRem(2))
	{
		sptr<Token> t = tkns.cur();
		
		if (t->type != TokenType::KEYWORD)
		{
			break;
		}

		sptr<Token> n = tkns.next();

		if (n->type != TokenType::IDENTIFIER)
		{
			auto e = errors->err("Names within generic signatures must be a valid identifier", n);
			break;
		}

		tkns.consume();

		GenericResult defRes;

		if (tkns.cur()->type == TokenType::SETTER)
		{
			tkns.consume();

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
				auto e = errors->err("Invalid generic default starts here", tkns.cur());
			}

		}
		else if (parsingDefaults)
		{
			auto e = errors->err("Generic signature defaults must always be at the end", tkns.cur());
		}

		auto genName = GenericName(t, n, defRes);

		names.push_back(genName);

		if (tkns.cur()->str != ",")
		{
			break;
		}

		tkns.consume();

	}

	sptr<Token> last = tkns.cur();

	if (last->str != GENERIC_END)
	{
		tkns.revertTo(start);
		return nullptr;
	}

	tkns.consume();

	auto sig = new_uptr<GenericSignature>(names);

	sig->first = first;
	sig->last = last;

	return sig;
}

sptr<GenericArguments> Parser::parseGenericArgs()
{
	if (!tkns.hasRem(3))
	{
		return nullptr;
	}

	sptr<Token> first = tkns.cur();
	auto start = tkns.index();
	
	if (first->str != GENERIC_START)
	{
		return new_sptr<GenericArguments>();
	}

	tkns.consume();

	std::vector<GenericResult> results;

	while (tkns.hasRem(2))
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

		if (tkns.cur()->str != ",")
		{
			break;
		}

		tkns.consume();

	}

	auto& last = tkns.cur();

	if (last->str != GENERIC_END)
	{
		tkns.revertTo(start);
		return nullptr;
	}

	tkns.consume();

	auto args = new_sptr<GenericArguments>(results);

	args->first = first;
	args->last = last;

	return args;
}

std::vector<sptr<Value>> Parser::parseValueList(bool commaOptional)
{
	std::vector<sptr<Value>> values;

	while (tkns.hasCur())
	{
		auto v = parseAnyValue();

		if (v == nullptr)
		{
			break;
		}

		values.push_back(v);

		if (tkns.cur()->str == ",")
		{
			tkns.consume();
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
	auto& end = tkns.cur();

	if (end->type == TokenType::END)
	{
		tkns.consume();
	}
	else
	{
		auto e = errors->err({ "Expected a semicolon, got", end->str }, end);
	}

}

bool Parser::parseScopeEnd(out<ScopeStatement> stmt)
{
	sptr<Token> first = tkns.cur();
	auto tknIndex = tkns.index();
	
	if (first->type != TokenType::KEYWORD)
	{
		return false;
	}

	tkns.consume();

	auto retMode = RETURN_MODES.find(first->str);

	if (retMode == RETURN_MODES.end())
	{
		auto e = errors->err("Not a valid statement or end of scope", first);

		tkns.revertTo(tknIndex);

		return false;
	}

	stmt.retMode = retMode->second;

	if (stmt.retMode == ReturnMode::RETURN)
	{
		stmt.retValue = parseAnyValue();
	}
	
	parseSemicolon();

	return true;
}

StmtModifiers Parser::parseStmtMods()
{
	StmtModifiers mods = {};

	while (tkns.hasCur())
	{
		auto& mod = tkns.cur();

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

		tkns.consume();

	}

	return mods;
}

uptr<ScopeStatement> Parser::parseScope(in<std::vector<ParseMethod<uptr<Statement>>>> pms, bool err)
{
	auto mods = parseStmtMods();

	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::START_SCOPE)
	{
		if (err)
		{
			auto e = errors->err("Expected a { here", first);

			e->note("Wrap your code in a scope");

		}
		
		return nullptr;
	}

	tkns.consume();

	auto scope = new_uptr<ScopeStatement>();

	scope->first = first;

	while (tkns.hasCur())
	{
		if (parseScopeEnd(*scope))
		{
			if (tkns.cur()->type != TokenType::END_SCOPE)
			{
				auto e = errors->err("Found an end-scope statement, but it did not end the scope", tkns.cur());
				continue;
			}

			scope->last = tkns.cur();

			tkns.consume();
			break;
		}

		if (tkns.cur()->type == TokenType::END_SCOPE)
		{
			tkns.consume();
			break;
		}

		for (auto const& pm : pms)
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
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::IDENTIFIER && first->str != "dynamic")
	{
		return nullptr;
	}

	tkns.consume();

	sptr<ParsedType> type = new_sptr<ParsedType>(first);

	type->genericArgs = parseGenericArgs();

	while (tkns.hasRem(2) && tkns.cur()->type == TokenType::START_BRACKET)
	{
		tkns.consume();

		auto arrayType = new_sptr<ParsedType>("array");

		auto gArgs = new_sptr<GenericArguments>();
		gArgs->args.push_back(type);

		if (tkns.cur()->type != TokenType::END_BRACKET)
		{
			if (auto len = parseAnyValue())
			{
				if (!len->isCompileTimeConst())
				{
					auto e = errors->err("Array length must be a compile-time constant", *len);
				}

				gArgs->args.push_back(len);
			}
			else
			{
				//TODO complain (oof)
			}

		}

		if (tkns.cur()->type != TokenType::END_BRACKET)
		{
			auto e = errors->err("Array types must end with a ]", tkns.cur());

		}

		type = arrayType;
		type->lastToken = tkns.cur();

		tkns.consume();

	}

	return type;
}

uptr<Statement> Parser::parseDecl()
{
	auto annotations = parseAllAnnotations();
	auto mods = parseStmtMods();

	sptr<Token> first = tkns.cur();

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
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::KEYWORD || first->str != "import")
	{
		return nullptr;
	}

	auto& modName = tkns.next();

	if (modName->type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Not a valid module name", modName);

		if (modName->type == TokenType::KEYWORD)
		{
			e->note("Can't import reserved keywords; try renaming your module");
		}

		if (modName->str == "*")
		{
			e->note("Can't import everything; try an actual module name");
		}

		return nullptr;
	}

	tkns.consume();

	auto ret = new_uptr<ImportStatement>(first);

	ret->name = modName;

	if (tkns.cur()->str == "as")
	{
		sptr<Token> alias = tkns.next();

		if (alias->type == TokenType::IDENTIFIER)
		{
			ret->alias = tkns.next();

			tkns.consume();

		}
		else
		{
			auto e = errors->err("Invalid import alias", alias);

			if (alias->str == "*")
			{
				e->note("Wildcard imports are currently not supported");
			}

		}

	}

	return ret;
}

uptr<Statement> Parser::parseModuleDef()
{
	auto& start = tkns.cur();

	if (start->type != TokenType::KEYWORD || start->str != "module")
	{
		return nullptr;
	}

	auto& name = tkns.next();

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
	if (!tkns.hasRem(5))
	{
		return nullptr;
	}

	auto& start = tkns.cur();

	if (start->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (start->str == "strong")
	{
		tkns.consume();
	}

	if (tkns.cur()->str != "type")
	{
		return nullptr;
	}

	auto& name = tkns.next();

	if (name->type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid type alias name", name);

		return nullptr;
	}

	if (tkns.next()->type != TokenType::SETTER)
	{
		auto e = errors->err("Type definitions must have an equal sign to denote the aliased type", tkns.cur());

		return nullptr;
	}

	sptr<Token> typeStart = tkns.next();

	if (sptr<ParsedType> aliasedType = parseTypeName())
	{
		return new_uptr<TypedefStatement>(settings, start, name, aliasedType);
	}
	else
	{
		auto e = errors->err("Invalid start of type name", typeStart);

		if (typeStart->str == "[")
		{
			e->note("Array types are in the C-style format of x[], where x is the element type.");
		}

		if (typeStart->type == TokenType::KEYWORD)
		{
			e->note("Only keyword that's a valid type is the dynamic keyword");
		}

		return nullptr;
	}

}

uptr<Statement> Parser::parseShader()
{
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::KEYWORD && first->str != "shader")
	{
		auto e = errors->err("Bruh.", first);
		return nullptr;
	}

	sptr<Token> name = tkns.next();

	if (name->type == TokenType::IDENTIFIER)
	{
		tkns.consume();
	}
	else
	{
		auto e = errors->err("Invalid shader object name", name);

		if (name->type == TokenType::START_SCOPE)
		{
			e->note("Name your shader so it can be found and compiled. Caliburn does not support 'anonymous' shaders");
		}

	}

	auto shader = new_uptr<ShaderStatement>();

	shader->first = first;
	shader->name = name;

	if (tkns.cur()->type == TokenType::START_PAREN)
	{
		tkns.consume();

		while (tkns.hasCur())
		{
			if (tkns.cur()->type == TokenType::END_PAREN)
			{
				tkns.consume();
				break;
			}

			auto type = parseTypeName();
			
			if (type == nullptr)
			{
				auto e = errors->err("Invalid descriptor type", tkns.cur());
				break;
			}

			sptr<Token> name = tkns.cur();

			if (name->type != TokenType::IDENTIFIER)
			{
				auto e = errors->err("Invalid descriptor name", tkns.cur());
				break;
			}

			tkns.consume();

			shader->descriptors.push_back(std::pair(type, name));

			if (tkns.cur()->type == TokenType::COMMA)
			{
				tkns.consume();
				continue;
			}
			
		}

	}

	if (tkns.cur()->type != TokenType::START_SCOPE)
	{
		auto e = errors->err("Shaders must start with a scope", tkns.cur());
		skipStmt();
		return shader;
	}

	tkns.consume();

	while (tkns.hasCur())
	{
		sptr<Token> memStart = tkns.cur();

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
				auto e = errors->err("Invalid shader object member; skipping statement", tkns.cur());
				
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

	while (tkns.cur()->type != TokenType::END_SCOPE)
	{
		auto e = errors->err("Stray token; skipping", tkns.cur());
		tkns.consume();
	}

	tkns.consume();

	return shader;
}

uptr<Statement> Parser::parseStruct()
{
	sptr<Token> first = tkns.cur();

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

	auto stmt = new_uptr<StructStatement>(stmtType);

	stmt->first = first;
	
	auto name = tkns.next();

	if (name->type == TokenType::IDENTIFIER)
	{
		tkns.consume();
	}
	else
	{
		auto e = errors->err("Invalid type name", name);

		if (name->type == TokenType::START_SCOPE)
		{
			e->note("Name your data structure");
		}

	}

	stmt->name = name;

	if (tkns.cur()->type == TokenType::START_SCOPE)
	{
		tkns.consume();
	}
	else
	{
		auto e = errors->err("Types must start with a scope", tkns.cur());
		skipStmt();
		return stmt;
	}

	while (true)
	{
		sptr<Token> end = tkns.cur();

		if (end->type == TokenType::END_SCOPE)
		{
			stmt->last = end;
			tkns.consume();
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
			std::string memName = fn->name->str;
			sptr<FunctionGroup> fnName = nullptr;

			if (auto foundName = stmt->members.find(memName); foundName != stmt->members.end())
			{
				if (auto fnNamePtr = std::get_if<sptr<FunctionGroup>>(&foundName->second))
				{
					fnName = *fnNamePtr;
				}
				else
				{
					//TODO complain
				}
			}
			else
			{
				fnName = new_sptr<FunctionGroup>();
				stmt->members.emplace(memName, fnName);
			}

			//TODO can't add functions to FunctionName without CLLR IDs

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
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (!FN_STARTS.count(first->str))
	{
		return nullptr;
	}

	tkns.consume();

	auto fn = new_uptr<ParsedFn>();

	fn->first = first;
	fn->type = FN_TYPES.at(first->str);

	if (fn->type == FnType::FUNCTION)
	{
		sptr<Token> gpuDim = tkns.cur();

		if (gpuDim->str == "[")
		{
			parseAnyBetween("[", LAMBDA(){
				fn->invokeDims = parseIdentifierList();
			}, "]");

			if (fn->invokeDims.size() == 0)
			{
				auto e = errors->err("Invocation dimensions are empty", gpuDim, tkns.cur());

				e->note("To use this feature, add identifiers separated with commas. Each identifier is a dimension within the GPU invocation (first is X, second is Y, etc.)");

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
		fn->name = tkns.cur();
		tkns.consume();

	}

	if (fn->type == FnType::FUNCTION)
	{
		sptr<Token> genSigStart = tkns.cur();

		fn->genSig = parseGenericSig();

		if (fn->genSig == nullptr)
		{
			auto e = errors->err("Invalid start to generic signature", genSigStart);
			return nullptr;
		}

	}
	
	parseAnyBetween("(", LAMBDA(){
		fn->args = parseFnArgs();
	}, ")");

	sptr<Token> hintStart = tkns.cur();

	if (hintStart->type == TokenType::COLON)
	{
		sptr<Token> typeNameStart = tkns.next();

		if (auto type = parseTypeName())
		{
			fn->returnType = type;
		}
		else
		{
			auto e = errors->err("Invalid return type after", hintStart);

			if (typeNameStart->str == "void")
			{
				e->note("Void is not a proper type name; leave out the return type altogether");
			}

			if (typeNameStart->str == "dynamic")
			{
				e->note("Make a typedef statement; you can't make a dynamic type a return type directly since it has no name to look up");
			}

		}

	}
	else
	{
		fn->returnType = new_sptr<ParsedType>("void");
	}

	sptr<Token> scopeStart = tkns.cur();
	
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
	auto start = tkns.index();

	auto as = parseAllAnnotations();

	if (tkns.cur()->type != TokenType::KEYWORD)
	{
		tkns.revertTo(start);
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
		tkns.revertTo(start);
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
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::KEYWORD || first->str != "if")
	{
		return nullptr;
	}

	tkns.consume();

	auto stmt = new_uptr<IfStatement>();

	stmt->first = first;

	parseAnyBetween("(", LAMBDA() {
		stmt->condition = parseAnyValue();
	}, ")");

	stmt->innerIf = parseScope({&Parser::parseDecl});

	if (tkns.cur()->str == "else")
	{
		sptr<Token> scopeStart = tkns.cur();

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
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::KEYWORD || first->str != "while")
	{
		return nullptr;
	}

	auto stmt = new_uptr<WhileStatement>();
	
	stmt->first = first;

	parseAnyBetween("(", LAMBDA() {
		stmt->condition = parseAnyValue();
	}, ")");

	stmt->loop = parseScope({ &Parser::parseLogic });

	return stmt;
}

uptr<Statement> Parser::parseDoWhile()
{
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::KEYWORD || first->str != "do")
	{
		return nullptr;
	}

	tkns.consume();

	auto ret = new_uptr<WhileStatement>();

	ret->first = first;
	ret->doWhile = true;
	ret->loop = parseScope({&Parser::parseLogic});

	sptr<Token> whileKwd = tkns.cur();

	if (whileKwd->type != TokenType::KEYWORD || whileKwd->str != "while")
	{
		auto e = errors->err("This is a do/while loop, where is your while keyword?", whileKwd);
		return nullptr;
	}

	tkns.consume();

	parseAnyBetween("(", LAMBDA() {
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
	sptr<Token> first = tkns.cur();

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
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (first->str != "const" && first->str != "var")
	{
		return nullptr;
	}

	auto stmt = new_uptr<VariableStatement>();

	stmt->first = tkns.cur();
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

sptr<Value> Parser::parseTerm()
{
	std::vector<ParseMethod<sptr<Value>>> initParsers =
		{ &Parser::parseParenValue, &Parser::parseUnaryValue, &Parser::parseLiteral, &Parser::parseAnyFnCall, &Parser::parseAnyAccess };

	sptr<Value> v = parseAny(initParsers);

	if (v == nullptr)
	{
		return nullptr;
	}

	while (tkns.hasRem(2)) //all of the below uses at least 2 tokens
	{
		sptr<Token> tkn = tkns.cur();

		if (tkn->type == TokenType::PERIOD && tkns.peek(1)->type == TokenType::IDENTIFIER)
		{
			tkns.consume();
			v = parseAccess(v);
		}
		else if (tkn->type == TokenType::START_BRACKET)
		{
			auto subA = new_sptr<SubArrayValue>();

			subA->array = v;

			parseAnyBetween("[", LAMBDA() {
				if (auto i = parseAnyValue())
				{
					subA->index = i;
				}
				else
				{
					auto e = errors->err("Invalid array index", tkns.cur());
				}
			}, "]");
			
			//parseAnyBetween() ends on the first invalid token, so we go back one to get the last token
			subA->last = tkns.peekBack(1);

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

	while (tkns.hasCur())
	{
		auto& keywd = tkns.cur();

		if (keywd->type != TokenType::KEYWORD)
		{
			break;
		}

		if (keywd->str == "is")
		{
			tkns.consume();

			auto isa = new_sptr<IsAValue>();

			isa->val = v;
			isa->chkType = parseTypeName();

			v = isa;

		}
		else if (keywd->str == "as")
		{
			tkns.consume();

			auto cast = new_sptr<CastValue>();

			cast->lhs = v;
			cast->castTarget = parseTypeName();

			v = cast;

		}
		else break;
	}

	return v;
}

sptr<Value> Parser::parseLiteral()
{
	sptr<Token> first = tkns.cur();

	tkns.consume();

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

		tkns.rewind();
		return nullptr;
	}

	if (first->type == TokenType::START_BRACKET)
	{
		auto arrLit = new_sptr<ArrayLitValue>();

		arrLit->start = first;
		
		parseAnyBetween("[", LAMBDA() {
			auto xs = parseValueList(true);

			if (xs.empty())
			{
				auto e = errors->err("Empty array literal", first, tkns.peek(1));

			}

			arrLit->values = xs;

		}, "]");
		
		return arrLit;
	}

	tkns.rewind();
	return nullptr;
}

sptr<Value> Parser::parseUnaryValue()
{
	sptr<Token> first = tkns.cur();

	if (first->type == TokenType::KEYWORD)
	{
		tkns.consume();

		if (first->str == "sign")
		{
			return new_sptr<SignValue>(first, parseTerm());
		}
		else if (first->str == "unsign")
		{
			return new_sptr<UnsignValue>(first, parseTerm());
		}

		tkns.rewind();
		return nullptr;
	}
	else if (first->type == TokenType::OPERATOR)
	{
		if (auto foundOp = UNARY_OPS.find(first->str); foundOp != UNARY_OPS.end())
		{
			tkns.consume();

			auto unary = new_sptr<UnaryValue>();

			unary->start = first;
			unary->op = foundOp->second;
			unary->val = parseTerm();
			
			if (foundOp->second == Operator::ABS)
			{
				if (!tkns.hasCur())
				{
					auto e = errors->err("Could not find end of absolute operator", first);
					
					return nullptr;
				}

				sptr<Token> absEnd = tkns.cur();

				if (absEnd->str != "|")
				{
					auto e = errors->err("Absolute operator values must end with |", absEnd);

					e->note("Unary operators, like abs, only looks for a single term. If you're trying to put an expression inside an abs op, put it in parentheses");
					e->note("Like so: |(x - 2)|");

					return nullptr;
				}

				unary->end = absEnd;

				tkns.consume();

			}

			return unary;
		}

	}

	return nullptr;
}

sptr<Value> Parser::parseParenValue()
{
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::START_PAREN)
	{
		return nullptr;
	}

	tkns.consume();

	auto v = parseAnyValue();

	//TODO tuple literals go here

	sptr<Token> last = tkns.cur();

	if (last->type != TokenType::END_PAREN)
	{
		auto e = errors->err("Expected a closing parentheses", last);
	}

	tkns.consume();

	return v;
}

sptr<Value> Parser::parseAnyAccess()
{
	return parseAccess(nullptr);
}

sptr<Value> Parser::parseAccess(sptr<Value> target)
{
	sptr<Token> first = tkns.cur();

	if (first->type != TokenType::IDENTIFIER)
	{
		return target;
	}

	auto access = new_sptr<VarChainValue>();

	access->target = target;
	
	while (true)
	{
		//check for method call
		if (tkns.hasRem(3))
		{
			auto& fwd = tkns.peek(1);

			if (fwd->str == GENERIC_START || fwd->type == TokenType::START_PAREN)
			{
				if (auto fnCall = parseFnCall(access->chain.empty() ? target : access))
				{
					return fnCall;
				}
				else
				{
					//TODO complain?
				}

			}

		}
		
		access->chain.push_back(tkns.cur());

		tkns.consume();

		if (!tkns.hasRem(2))
		{
			break;
		}

		if (tkns.peek(1)->type != TokenType::PERIOD)
		{
			break;
		}

		if (tkns.peek(2)->type != TokenType::IDENTIFIER)
		{
			break;
		}

		tkns.consume(2);

	}

	return access;
}

sptr<Value> Parser::parseExpr()
{
	auto start = parseTerm();

	if (start == nullptr)
	{
		return nullptr;
	}

	std::stack<sptr<Value>> values;
	std::stack<Operator> ops;

	values.push(start);
	
	const auto makeExpr = LAMBDA()
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

	while (tkns.hasRem(2))
	{
		//size_t last = tkns.currentIndex();

		sptr<Token> opTkn = tkns.cur();
		sptr<Value> term = nullptr;
		auto op = Operator::NONE;
		
		if (opTkn->type == TokenType::OPERATOR)
		{
			if (auto found = INFIX_OPS.find(opTkn->str); found != INFIX_OPS.end())
			{
				tkns.consume();
				op = found->second;
				term = parseTerm();
			}
			else break;
		}
		else if (opTkn->type == TokenType::START_PAREN)
		{
			op = Operator::MUL;
			term = parseParenValue();
		}
		else break;

		if (term == nullptr)
		{
			auto e = errors->err("Expected a value after here", opTkn);
			break;
		}

		if (!ops.empty() && OP_PRECEDENCE.at(ops.top()) >= OP_PRECEDENCE.at(op))
		{
			makeExpr();
		}

		ops.push(op);
		values.push(term);

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
	if (tkns.hasRem(3))
	{
		return nullptr;
	}

	sptr<Token> name = tkns.cur();

	if (name->type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	tkns.consume();

	auto fnCall = new_sptr<FnCallValue>();

	fnCall->name = name;
	fnCall->target = start;

	sptr<Token> genArgStart = tkns.cur();

	fnCall->genArgs = parseGenericArgs();

	if (fnCall->genArgs == nullptr)
	{
		auto e = errors->err("Invalid generic argument start", genArgStart);

		return nullptr;
	}

	if (!parseAnyBetween("(", LAMBDA() {
		fnCall->args = parseValueList(false);
	}, ")"))
	{
		return nullptr;
	}

	fnCall->end = tkns.peekBack(1);

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
	sptr<Token> first = tkns.cur();

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

	if (tkns.cur()->str == ":")
	{
		v->typeHint = parseTypeName();
	}

	sptr<Token> name = tkns.cur();
	
	if (name->type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid global variable name", name);
		return nullptr;
	}

	v->nameTkn = name;

	sptr<Token> eqSign = tkns.next();

	if (eqSign->type != TokenType::SETTER)
	{
		auto e = errors->err("Global variables must be initialized", eqSign);

		e->note("Add an equal sign and a value");

		return v;
	}

	tkns.consume();

	if (auto init = parseAnyValue())
	{
		if (!init->isCompileTimeConst())
		{
			auto e = errors->err("Global variables initialized with a non-constant value", *init);

			e->note("Use a compile-time constant, like a literal. Struct-likes do not count.");

		}

		v->initValue = init;

	}
	else
	{
		auto e = errors->err("Could not find an initial value here", tkns.cur());
	}

	return v;
}

sptr<Variable> Parser::parseMemberVar()
{
	auto mods = parseStmtMods();

	sptr<Token> start = tkns.cur();
	bool isConst = false;
	
	if (start->type == TokenType::KEYWORD && start->str == "const")
	{
		isConst = true;
		tkns.consume();
	}

	sptr<Token> typeStart = tkns.cur();
	auto type = parseTypeName();

	if (type == nullptr)
	{
		auto e = errors->err("Invalid member variable type name starts here", typeStart);

		return nullptr;
	}

	sptr<Token> name = tkns.cur();
	
	if (name->type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid member variable name", name);

		return nullptr;
	}

	tkns.consume();

	auto v = new_sptr<MemberVariable>();

	v->mods = mods;
	v->first = start;
	v->nameTkn = name;
	v->typeHint = type;
	v->isConst = isConst;

	if (tkns.cur()->type == TokenType::SETTER)
	{
		tkns.consume();

		v->initValue = parseAnyValue();
	}

	return v;
}

std::vector<sptr<ParsedVar>> Parser::parseLocalVarLike()
{
	std::vector<sptr<ParsedVar>> vars;
	sptr<Token> first = tkns.cur();

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

	tkns.consume();

	sptr<ParsedType> typeHint = nullptr;

	if (tkns.cur()->str == ":")
	{
		tkns.consume();

		if (auto t = parseTypeName())
		{
			typeHint = t;
		}
		else
		{
			sptr<Token> typeName = tkns.cur();
			auto e = errors->err("Invalid type name", typeName);
		}

	}

	auto varNames = parseIdentifierList();

	if (varNames.empty())
	{
		sptr<Token> varName = tkns.cur();
		auto e = errors->err("Invalid local variable name", varName);
	}

	tkns.consume();

	if (tkns.cur()->str == ":")
	{
		auto e = errors->err("That's not how type hints work in Caliburn. Type hints go before the variable name, not after. So var: int x is valid. var x: int isn't.", tkns.cur());

		return vars;
	}

	sptr<Value> initValue = nullptr;

	if (tkns.cur()->type == TokenType::SETTER)
	{
		sptr<Token> valStart = tkns.next();

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
		auto e = errors->err("Implicitly-typed variables must be manually initialized", first, tkns.cur());
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
	sptr<Token> first = tkns.cur();

	if (first->type == TokenType::KEYWORD && first->str == "const")
	{
		isConst = true;
		tkns.consume();
	}

	auto typeHint = parseTypeName();

	if (typeHint == nullptr)
	{
		return vars;
	}

	auto varNames = parseIdentifierList();

	if (varNames.empty())
	{
		auto e = errors->err("Invalid member variable name", tkns.cur());
		return vars;
	}

	sptr<Value> initValue = nullptr;

	if (tkns.cur()->type == TokenType::SETTER)
	{
		if (auto v = parseAnyValue())
		{
			initValue = v;

		}
		else
		{
			auto e = errors->err("Invalid member variable initial value", tkns.cur());
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

	while (tkns.hasCur())
	{
		auto argType = parseTypeName();

		if (argType == nullptr)
		{
			break;
		}

		auto arg = new_uptr<ParsedFnArg>();

		arg->typeHint = argType;
		arg->name = tkns.cur();

		if (arg->name->type != TokenType::IDENTIFIER)
		{
			errors->err("Not a valid argument name", arg->name);
			break;
		}

		tkns.consume();

		fnArgs.push_back(std::move(arg));

		if (tkns.cur()->type == TokenType::COMMA)
		{
			tkns.consume();
			continue;
		}

		break;
	}

	return fnArgs;
}

std::vector<uptr<Annotation>> Parser::parseAllAnnotations()
{
	std::vector<uptr<Annotation>> vec;

	while (tkns.hasCur())
	{
		if (auto a = parseAnnotation())
		{
			vec.push_back(std::move(a));
		}
		else break;
	}

	return vec;
}

uptr<Annotation> Parser::parseAnnotation()
{
	if (!tkns.hasRem(2))
	{
		return nullptr;
	}

	sptr<Token> first = tkns.cur();
	sptr<Token> name = tkns.peek(1);

	if (first->str != "@")
	{
		return nullptr;
	}

	if (name->type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Annotations must be named with an identifier", first, name);
		return nullptr;
	}

	tkns.consume(2);

	auto a = new_uptr<Annotation>();

	a->first = first;
	a->name = name;

	sptr<Token> startParen = tkns.cur();

	if (startParen->type != TokenType::START_PAREN)
	{
		a->last = a->name;
		return a;
	}

	tkns.consume();
	int closingParenNeeded = 1;

	while (tkns.hasCur())
	{
		auto& tkn = tkns.cur();

		if (tkn->type == TokenType::END_PAREN)
		{
			--closingParenNeeded;

			if (closingParenNeeded == 0)
			{
				tkns.consume();
				break;
			}

		}
		else if (tkn->type == TokenType::START_PAREN)
		{
			++closingParenNeeded;
		}

		a->contents.push_back(tkn);
		tkns.consume();

	}

	if (closingParenNeeded != 0)
	{
		auto e = errors->err("Ran out of symbols and could not close annotation", startParen);
	}

	a->last = tkns.cur();

	tkns.consume();

	return a;
}
