
#include "parser.h"

#include <stack>

#include "ast/ctrlstmt.h"
#include "ast/fnstmt.h"
#include "ast/generics.h"
#include "ast/modstmts.h"
#include "ast/setstmt.h"
#include "ast/shaderstmt.h"
#include "ast/structstmt.h"
#include "ast/typestmt.h"
#include "ast/values.h"
#include "ast/varstmt.h"

using namespace caliburn;

void Parser::skipStmt()
{
	size_t scopes = 0;

	while (tkns.hasCur())
	{
		auto const& tkn = tkns.cur();

		if (tkn.type == TokenType::START_SCOPE)
		{
			++scopes;
		}
		else if (tkn.type == TokenType::END_SCOPE)
		{
			if (scopes > 0)
			{
				--scopes;
			}
		}
		
		tkns.consume();

		if (scopes == 0 && tkn.type == TokenType::END)
		{
			break;
		}

	}

}

std::vector<sptr<Expr>> Parser::parse()
{
	std::vector<sptr<Expr>> ast;

	while (tkns.hasCur())
	{
		auto const start = tkns.cur();

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
	size_t const current = tkns.offset();

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
	if (!tkns.hasCur())
	{
		return false;
	}

	auto const startTkn = tkns.cur();

	if (startTkn.str != start)
	{
		return false;
	}

	tkns.consume();

	fn();

	auto const endTkn = tkns.cur();

	if (endTkn.str != end)
	{
		auto e = errors->err({ "Unexpected token found; Expected", end }, startTkn, endTkn);
		return false;
	}

	tkns.consume();

	return true;
}

std::vector<Token> Parser::parseIdentifierList()
{
	std::vector<Token> ids;

	while (true)
	{
		if (tkns.cur().type != TokenType::IDENTIFIER)
		{
			break;
		}

		ids.push_back(tkns.cur());

		if (!tkns.hasRem(2))
		{
			break;
		}

		if (tkns.next().type != TokenType::COMMA)
		{
			break;
		}

		tkns.consume();

	}

	return ids;
}

uptr<GenericSignature> Parser::parseGenericSig()
{
	auto const first = tkns.cur();
	size_t const start = tkns.offset();
	
	if (first.str != GENERIC_START)
	{
		//We return an empty signature since every generic needs one, even a blank one.
		return new_uptr<GenericSignature>();
	}

	std::vector<GenericName> names;

	bool parsingDefaults = false;

	while (tkns.hasRem(2))
	{
		auto const t = tkns.cur();
		
		if (t.type != TokenType::KEYWORD)
		{
			break;
		}

		auto const n = tkns.next();

		if (n.type != TokenType::IDENTIFIER)
		{
			auto e = errors->err("Names within generic signatures must be a valid identifier", n);
			break;
		}

		tkns.consume();

		GenericResult defRes;

		if (tkns.cur().type == TokenType::SETTER)
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

		if (tkns.cur().type != TokenType::COMMA)
		{
			break;
		}

		tkns.consume();

	}

	auto const& last = tkns.cur();

	if (last.str != GENERIC_END)
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

	auto const first = tkns.cur();
	size_t const start = tkns.offset();
	
	if (first.str != GENERIC_START)
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

		if (tkns.cur().type != TokenType::COMMA)
		{
			break;
		}

		tkns.consume();

	}

	auto const& last = tkns.cur();

	if (last.str != GENERIC_END)
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

std::vector<sptr<Expr>> Parser::parseValueList()
{
	std::vector<sptr<Expr>> values;

	while (tkns.hasCur())
	{
		auto v = parseExpr();

		if (v == nullptr)
		{
			break;
		}

		if (tkns.cur().type == TokenType::COMMA)
		{
			tkns.consume();
			continue;
		}
		else break;

	}

	return values;
}

bool Parser::parseSemicolon()
{
	if (!tkns.hasCur())
	{
		return false;
	}

	if (tkns.cur().type != TokenType::END)
	{
		auto e = errors->err("Expected semicolon after here", tkns.peekBack(1));
		return false;
	}

	tkns.consume();
	return true;
}

ExprModifiers Parser::parseStmtMods()
{
	ExprModifiers mods = {};

	while (tkns.hasCur())
	{
		auto const& mod = tkns.cur();

		if (mod.type != TokenType::KEYWORD)
		{
			break;
		}

		if (mod.str == "public")
			mods.PUBLIC = 1;
		else if (mod.str == "private")
			mods.PRIVATE = 1;
		else if (mod.str == "shared")
			mods.SHARED = 1;
		else if (mod.str == "static")
			mods.STATIC = 1;
		else break;

		tkns.consume();

	}

	return mods;
}

uptr<ScopeStmt> Parser::parseScope(in<std::vector<ParseMethod<sptr<Expr>>>> pms, bool err)
{
	auto const mods = parseStmtMods();

	auto const first = tkns.cur();

	if (first.type != TokenType::START_SCOPE)
	{
		if (err)
		{
			auto e = errors->err("Expected a { here", first);

			e->note("Wrap your code in a scope");

		}
		
		return nullptr;
	}

	tkns.consume();

	auto scope = new_uptr<ScopeStmt>();

	scope->first = first;

	while (tkns.hasCur())
	{
		if (tkns.cur().type == TokenType::END_SCOPE)
		{
			break;
		}

		bool parsed = false;

		for (auto const& pm : pms)
		{
			auto const off = tkns.offset();

			if (auto stmt = pm(*this))
			{
				parsed = true;

				stmt->mods = mods;

				if (!parseSemicolon())
				{
					auto e = errors->err("Expected a semicolon here", tkns.hasCur() ? tkns.cur() : tkns.last());
					skipStmt();
				}

				scope->stmts.push_back(std::move(stmt));

				break;
			}
			else
			{
				tkns.revertTo(off);
			}

		}

		if (!parsed)
		{
			break;
		}

	}

	if (!tkns.hasCur())
	{
		auto e = errors->err("Unexpected end of scope", tkns.last());
		return nullptr;
	}

	if (tkns.cur().type != TokenType::END_SCOPE)
	{
		auto e = errors->err("Expected end of scope here", tkns.cur());
		return scope;
	}

	scope->last = tkns.cur();

	tkns.consume();
	return scope;
}

sptr<ParsedType> Parser::parseTypeName()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::IDENTIFIER && first.str != "dynamic")
	{
		return nullptr;
	}

	tkns.consume();

	auto type = new_sptr<ParsedType>(first, parseGenericArgs());

	while (tkns.hasRem(2) && tkns.cur().type == TokenType::START_BRACKET)
	{
		tkns.consume();

		auto arrayType = new_sptr<ParsedType>("array");

		auto gArgs = new_sptr<GenericArguments>();
		gArgs->args.push_back(type);

		if (tkns.cur().type != TokenType::END_BRACKET)
		{
			if (auto len = parseExpr())
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

		if (tkns.cur().type != TokenType::END_BRACKET)
		{
			auto e = errors->err("Array types must end with a ]", tkns.cur());

		}

		type = arrayType;
		type->lastToken = tkns.cur();

		tkns.consume();

	}

	return type;
}

sptr<Expr> Parser::parseDecl()
{
	auto annotations = parseAllAnnotations();
	auto const mods = parseStmtMods();

	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	std::vector<ParseMethod<sptr<Expr>>> methods = {
		&Parser::parseImport,
		&Parser::parseModuleDef,
		&Parser::parseGlobalVarStmt,
		&Parser::parseTypedef,
		&Parser::parseFnStmt,
		&Parser::parseShader,
		&Parser::parseStruct,
		&Parser::parseTopLevelIf
	};

	auto stmt = parseAny(methods);

	if (stmt == nullptr)
	{
		return stmt;
	}

	for (auto& a : annotations)
	{
		stmt->annotations.emplace(a->name.str, std::move(a));
	}

	stmt->mods = mods;
	
	if (!parseSemicolon())
	{
		auto e = errors->err("Expected a semicolon after end of declaration", tkns.cur());
	}

	return stmt;
}

sptr<Expr> Parser::parseImport()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD || first.str != "import")
	{
		return nullptr;
	}

	auto const modName = tkns.next();

	if (modName.type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Not a valid module name", modName);

		if (modName.type == TokenType::KEYWORD)
		{
			e->note("Can't import reserved keywords; try renaming your module");
		}

		if (modName.str == "*")
		{
			e->note("Can't import everything; try an actual module name");
		}

		return nullptr;
	}

	tkns.consume();

	if (tkns.cur().str != "as")
	{
		return new_uptr<ImportStmt>(first, modName);
	}

	auto const alias = tkns.next();

	if (alias.type == TokenType::IDENTIFIER)
	{
		tkns.consume();

		return new_uptr<ImportStmt>(first, modName, alias);
	}
	
	auto e = errors->err("Invalid import alias", alias);

	if (alias.str == "*")
	{
		e->note("Wildcard imports are currently not supported");
	}

	return nullptr;
}

sptr<Expr> Parser::parseModuleDef()
{
	auto const start = tkns.cur();

	if (start.type != TokenType::KEYWORD || start.str != "module")
	{
		return nullptr;
	}

	auto const name = tkns.next();

	if (name.type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid module name", name);
		return nullptr;
	}

	return new_uptr<ModuleStmt>(start, name);
}

sptr<Expr> Parser::parseTypedef()
{
	//type x = y;
	if (!tkns.hasRem(5))
	{
		return nullptr;
	}

	auto const start = tkns.cur();

	if (start.type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (start.str == "strong")
	{
		tkns.consume();
	}

	if (tkns.cur().str != "type")
	{
		return nullptr;
	}

	auto const name = tkns.next();

	if (name.type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid type alias name", name);

		return nullptr;
	}

	if (tkns.next().type != TokenType::SETTER)
	{
		auto e = errors->err("Type definitions must have an equal sign to denote the aliased type", tkns.cur());

		return nullptr;
	}

	auto const typeStart = tkns.next();

	if (sptr<ParsedType> aliasedType = parseTypeName())
	{
		return new_uptr<TypedefStmt>(settings, start, name, aliasedType);
	}
	else
	{
		auto e = errors->err("Invalid start of type name", typeStart);

		if (typeStart.str == "[")
		{
			e->note("Array types are in the C-style format of x[], where x is the element type.");
		}

		if (typeStart.type == TokenType::KEYWORD)
		{
			e->note("Only keyword that's a valid type is the dynamic keyword");
		}

	}

	return nullptr;
}

sptr<Expr> Parser::parseShader()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD && first.str != "shader")
	{
		return nullptr;
	}

	auto const name = tkns.next();

	if (name.type == TokenType::IDENTIFIER)
	{
		tkns.consume();
	}
	else
	{
		auto e = errors->err("Invalid shader object name", name);

		if (name.type == TokenType::START_SCOPE)
		{
			e->note("Name your shader so it can be found and compiled. Caliburn does not support 'anonymous' shaders");
		}

	}

	auto shader = new_uptr<ShaderStmt>();

	shader->first = first;
	shader->name = name;

	if (tkns.cur().type == TokenType::START_PAREN)
	{
		tkns.consume();

		while (tkns.hasCur())
		{
			if (tkns.cur().type == TokenType::END_PAREN)
			{
				tkns.consume();
				break;
			}

			auto const type = parseTypeName();
			
			if (type == nullptr)
			{
				auto e = errors->err("Invalid descriptor type", tkns.cur());
				break;
			}

			auto const name = tkns.cur();

			if (name.type != TokenType::IDENTIFIER)
			{
				auto e = errors->err("Invalid descriptor name", tkns.cur());
				break;
			}

			tkns.consume();

			shader->descriptors.push_back(std::pair(type, name));

			if (tkns.cur().type == TokenType::COMMA)
			{
				tkns.consume();
				continue;
			}
			
		}

	}

	if (tkns.cur().type != TokenType::START_SCOPE)
	{
		auto e = errors->err("Shaders must start with a scope", tkns.cur());
		skipStmt();
		return shader;
	}

	tkns.consume();

	while (tkns.hasCur())
	{
		auto const memStart = tkns.cur();

		if (memStart.type == TokenType::END_SCOPE)
		{
			break;
		}

		if (auto stageFn = parseFn())
		{
			shader->stages.push_back(new_uptr<ShaderStage>(stageFn, memStart, shader->name));

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

			for (auto const ioData : ios)
			{
				shader->ioVars.push_back(new_sptr<ShaderIOVariable>(*ioData));

			}

		}

		if (!parseSemicolon())
		{
			auto e = errors->err("Expected semicolon after shader member", tkns.cur());
		}

	}

	if (tkns.cur().type != TokenType::END_SCOPE)
	{
		auto e = errors->err("End of shader definition should be here", tkns.cur());
		return nullptr;
	}

	tkns.consume();

	return shader;
}

sptr<Expr> Parser::parseStruct()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	auto stmtType = ExprType::STRUCT;

	if (first.str == "record")
	{
		stmtType = ExprType::RECORD;
	}
	else if (first.str == "class")
	{
		stmtType = ExprType::CLASS;
	}
	else if (first.str != "struct")
	{
		return nullptr;
	}

	auto const& name = tkns.next();

	if (name.type == TokenType::IDENTIFIER)
	{
		tkns.consume();
	}
	else
	{
		auto e = errors->err("Invalid type name", name);

		if (name.type == TokenType::START_SCOPE)
		{
			e->note("Name your data structure");
		}

	}

	auto stmt = new_uptr<StructStmt>(stmtType, first, name);

	tkns.consume();

	if (tkns.cur().type == TokenType::START_SCOPE)
	{
		tkns.consume();
	}
	else
	{
		auto e = errors->err("Types must start with a scope", tkns.cur());
		skipStmt();
		return stmt;
	}

	if (!parseAnyBetween("{", LAMBDA() {
		while (tkns.hasCur())
		{
			auto const end = tkns.cur();

			if (end.type == TokenType::END_SCOPE)
			{
				stmt->last = end;
				break;
			}

			if (auto v = parseMemberVar())
			{
				if (stmtType == ExprType::RECORD)
				{
					v->isConst = true;
				}

				stmt->members.emplace(v->name.str, v);

			}
			else if (auto fn = parseFn())
			{
				stmt->memberFns.push_back(std::move(fn));
			}
			else
			{
				auto e = errors->err("Invalid start to type member", end);
			}

			if (!parseSemicolon())
			{
				auto e = errors->err("Expected semicolon after member", tkns.last());
			}

		}
	}, "}"))
	{
		auto e = errors->err("Failed to properly parse struct contents", first);
	}

	return stmt;
}

sptr<Expr> Parser::parseFnStmt()
{
	if (auto const fn = parseFn())
	{
		return new_uptr<FnStmt>(*fn);
	}

	return nullptr;
}

sptr<Expr> Parser::parseTopLevelIf()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD || first.str != "if")
	{
		return nullptr;
	}

	tkns.consume();

	auto stmt = new_uptr<IfStatement>();

	stmt->first = first;
	stmt->condition = parseExpr();
	stmt->innerIf = parseScope({ &Parser::parseDecl });

	if (tkns.cur().str == "else")
	{
		auto const& scopeStart = tkns.cur();

		stmt->innerElse = parseScope({ &Parser::parseDecl });

		if (stmt->innerElse == nullptr)
		{
			//TODO complain
		}

	}

	return stmt;
}

uptr<ParsedFn> Parser::parseFn()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (!FN_STARTS.count(first.str))
	{
		return nullptr;
	}

	tkns.consume();

	auto fn = new_uptr<ParsedFn>();

	fn->first = first;
	fn->type = FN_TYPES.at(first.str);

	if (fn->type == FnType::FUNCTION)
	{
		auto const& gpuDim = tkns.cur();

		if (gpuDim.str == "[")
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
		auto const genSigStart = tkns.cur();

		fn->genSig = parseGenericSig();

		if (fn->genSig == nullptr)
		{
			auto e = errors->err("Invalid start to generic signature", genSigStart);
			return nullptr;
		}

	}
	
	if (!parseAnyBetween("(", LAMBDA(){
		fn->args = parseFnArgs();
	}, ")"))
	{
		return nullptr;
	}

	auto const hintStart = tkns.cur();

	if (hintStart.type == TokenType::COLON)
	{
		auto const typeNameStart = tkns.next();

		if (auto type = parseTypeName())
		{
			fn->returnType = type;
		}
		else
		{
			auto e = errors->err("Invalid return type after", hintStart);

			if (typeNameStart.str == "void")
			{
				e->note("Void is not a proper type name; leave out the return type altogether");
			}

			if (typeNameStart.str == "dynamic")
			{
				e->note("Make a typedef statement; you can't make a dynamic type a return type directly since it has no name to look up");
			}

		}

	}
	else
	{
		fn->returnType = new_sptr<ParsedType>("void");
	}

	auto const& scopeStart = tkns.cur();
	
	if (scopeStart.type == TokenType::START_SCOPE)
	{
		fn->code = parseScope({ &Parser::parseLogic });
	}
	else
	{
		auto e = errors->err("Invalid start to function scope", scopeStart);
	}

	return fn;
}

sptr<Expr> Parser::parseLogic()
{
	return parseAny(std::vector<ParseMethod<sptr<Expr>>> {
		&Parser::parseControl,
		&Parser::parseSetter
	});
}

sptr<Expr> Parser::parseSetter()
{
	sptr<Expr> lhs = parseExpr();

	if (lhs == nullptr)
	{
		return nullptr;
	}

	Operator op = Operator::NONE;
	sptr<Expr> rhs = nullptr;

	if (tkns.cur().type == TokenType::OPERATOR)
	{
		if (auto opIdx = INFIX_OPS.find(tkns.cur().str); opIdx != INFIX_OPS.end())
		{
			auto op = opIdx->second;
			tkns.consume();
		}
		else
		{
			//TODO complain
		}

	}
	
	if (tkns.cur().type != TokenType::SETTER)
	{
		if (op != Operator::NONE)
		{
			//TODO complain
		}

		return lhs;
	}

	tkns.consume();

	rhs = parseExpr();

	if (rhs == nullptr)
	{
		//TODO complain
	}

	if (op != Operator::NONE)
	{
		rhs = new_sptr<ExpressionValue>(lhs, op, rhs);
	}

	return new_sptr<SetStmt>(lhs, rhs);
}

sptr<Expr> Parser::parseControl()
{
	size_t const start = tkns.offset();

	auto as = parseAllAnnotations();

	if (tkns.cur().type != TokenType::KEYWORD)
	{
		tkns.revertTo(start);
		return nullptr;
	}
	
	std::vector<ParseMethod<sptr<Expr>>> methods = {
		&Parser::parseLogicalIf,
		&Parser::parseWhile,
		&Parser::parseDoWhile,
		&Parser::parseScopeEnd,
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
			ctrl->annotations.emplace(a->name.str, std::move(a));
		}

	}

	return ctrl;
}

sptr<Expr> Parser::parseScopeEnd()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	tkns.consume();

	if (first.str == "return")
	{
		auto retStmt = new_uptr<ReturnStmt>(first);

		if (auto v = parseExpr())
		{
			retStmt->retValue = v;
		}

		return retStmt;
	}

	if (first.str == "break")
	{
		return new_uptr<BreakStmt>(first);
	}

	if (first.str == "continue")
	{
		return new_uptr<ContinueStmt>(first);
	}

	if (first.str == "discard")
	{
		return new_uptr<DiscardStmt>(first);
	}

	if (first.str == "pass")
	{
		return new_uptr<PassStmt>(first);
	}

	if (first.str == "unreachable")
	{
		return new_uptr<UnreachableStmt>(first);
	}

	tkns.rewind();

	return nullptr;
}

sptr<Expr> Parser::parseLogicalIf()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD || first.str != "if")
	{
		return nullptr;
	}

	tkns.consume();

	auto stmt = new_uptr<IfStatement>();

	stmt->first = first;
	stmt->condition = parseExpr();
	stmt->innerIf = parseScope({ &Parser::parseLogic });

	if (tkns.cur().str == "else")
	{
		auto const& scopeStart = tkns.cur();

		stmt->innerElse = parseScope({ &Parser::parseLogic });

		if (stmt->innerElse == nullptr)
		{
			//TODO complain
		}

	}

	return stmt;
}

sptr<Expr> Parser::parseFor() //FIXME
{
	return nullptr;
}

sptr<Expr> Parser::parseWhile()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD || first.str != "while")
	{
		return nullptr;
	}

	auto stmt = new_uptr<WhileStatement>();
	
	stmt->first = first;
	stmt->condition = parseExpr();
	stmt->loop = parseScope({ &Parser::parseLogic });

	return stmt;
}

sptr<Expr> Parser::parseDoWhile()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD || first.str != "do")
	{
		return nullptr;
	}

	tkns.consume();

	auto ret = new_uptr<WhileStatement>();

	ret->first = first;
	ret->doWhile = true;
	ret->loop = parseScope({&Parser::parseLogic});

	auto const& whileKwd = tkns.cur();

	if (whileKwd.type != TokenType::KEYWORD || whileKwd.str != "while")
	{
		auto e = errors->err("This is a do/while loop, where is your while keyword?", whileKwd);
		return nullptr;
	}

	tkns.consume();

	ret->condition = parseExpr();

	return ret;
}

sptr<Expr> Parser::parseGlobalVarStmt()
{
	auto const first = tkns.cur();

	if (auto v = parseGlobalVar())
	{
		auto stmt = new_uptr<VarStmt>();

		stmt->first = first;
		stmt->vars.push_back(v);

		return stmt;
	}

	return nullptr;
}

sptr<Expr> Parser::parseLocalVarStmt()
{
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (first.str != "const" && first.str != "var")
	{
		return nullptr;
	}

	auto stmt = new_uptr<VarStmt>();

	stmt->first = tkns.cur();
	stmt->vars = parseLocalVars();

	return stmt;
}

sptr<Expr> Parser::parseExpr()
{
	auto start = parseTerm();

	if (start == nullptr)
	{
		return nullptr;
	}

	std::stack<sptr<Expr>> values;
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

		auto const opTkn = tkns.cur();
		sptr<Expr> term = nullptr;
		auto op = Operator::NONE;

		if (opTkn.type == TokenType::OPERATOR)
		{
			if (auto found = INFIX_OPS.find(opTkn.str); found != INFIX_OPS.end())
			{
				tkns.consume();
				op = found->second;
				term = parseTerm();
			}
			else break;
		}
		else if (opTkn.type == TokenType::START_PAREN)
		{
			if (auto v = parseParenValue())
			{
				op = Operator::MUL;
				term = v;
			}
			else break;
		}
		else break;

		if (term == nullptr)
		{
			if (tkns.cur().type != TokenType::SETTER)
			{
				auto e = errors->err("Expected a value after here", opTkn);
			}

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

sptr<Expr> Parser::parseTerm()
{
	std::vector<ParseMethod<sptr<Expr>>> initParsers =
		{ &Parser::parseParenValue, &Parser::parseUnaryValue, &Parser::parseLiteral, &Parser::parseAccess };

	sptr<Expr> v = parseAny(initParsers);

	if (v == nullptr)
	{
		return nullptr;
	}

	while (tkns.hasRem(2)) //all of the below uses at least 2 tokens
	{
		auto const tkn = tkns.cur();

		if (tkn.type == TokenType::END)
		{
			return v;
		}

		if (tkn.type == TokenType::START_PAREN || tkn.str == GENERIC_START)
		{
			v = parseFnCall(v);
		}
		else if (tkn.type == TokenType::PERIOD)
		{
			v = parseMemberAccess(v);
		}
		else if (tkn.type == TokenType::START_BRACKET)
		{
			sptr<Expr> index = nullptr;
			
			if (!parseAnyBetween("[", LAMBDA() {
				if (auto i = parseExpr())
				{
					index = i;
				}
				else
				{
					auto e = errors->err("Invalid array index", tkns.cur());
				}
			}, "]"))
			{
				return nullptr;
			}
			
			//parseAnyBetween() ends on the first invalid token, so we go back one to get the last token
			v = new_sptr<SubArrayValue>(v, index, tkns.peekBack(1));

		}
		else
		{
			break;
		}

		if (v == nullptr)
		{
			auto e = errors->err("Parsing a value failed here; see other errors", tkn);
			break;
		}
		
	}

	while (tkns.hasRem(2))
	{
		auto& keywd = tkns.cur();

		if (keywd.type != TokenType::KEYWORD)
		{
			break;
		}

		if (keywd.str == "as")
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

sptr<Expr> Parser::parseLiteral()
{
	auto const& first = tkns.cur();

	tkns.consume();

	switch (first.type)
	{
		case TokenType::LITERAL_INT: return new_sptr<IntLiteralValue>(first);
		case TokenType::LITERAL_FLOAT: return new_sptr<FloatLiteralValue>(first);
		case TokenType::LITERAL_BOOL: return new_sptr<BoolLitValue>(first);
		case TokenType::LITERAL_STR: return new_sptr<StringLitValue>(first);
	}

	if (first.type == TokenType::KEYWORD)
	{
		if (first.str == "this")
		{
			return new_sptr<VarReadValue>(first);
		}
		else if (first.str == "null")
		{
			return new_sptr<NullValue>(first);
		}

		tkns.rewind();
		return nullptr;
	}

	if (first.type == TokenType::START_BRACKET)
	{
		auto arrLit = new_sptr<ArrayLitValue>();

		arrLit->start = first;

		tkns.consume();

		bool delimited = false;

		std::vector<sptr<Expr>> xs;

		while (tkns.hasCur())
		{
			if (tkns.cur().type == TokenType::END_BRACKET)
			{
				delimited = true;
				break;
			}

			if (auto v = parseExpr())
			{
				xs.push_back(v);
				continue;
			}
			else if (tkns.cur().type == TokenType::COMMA)
			{
				tkns.consume();
				continue;
			}
			else break;
			
		}

		if (!delimited)
		{
			if (tkns.hasCur())
			{
				auto e = errors->err("Array literal not ended with a ']'", arrLit->start, tkns.cur());
			}
			else
			{
				auto e = errors->err("Unexpected EOF; Array literal not ended with a ']'", arrLit->start, tkns.last());
			}

			return nullptr;
		}

		arrLit->values = xs;
		arrLit->end = tkns.take();

		return arrLit;
	}

	tkns.rewind();
	return nullptr;
}

sptr<Expr> Parser::parseUnaryValue()
{
	auto const& first = tkns.cur();

	if (first.type == TokenType::KEYWORD)
	{
		tkns.consume();

		if (first.str == "sign")
		{
			return new_sptr<SignValue>(first, parseTerm());
		}
		else if (first.str == "unsign")
		{
			return new_sptr<UnsignValue>(first, parseTerm());
		}

		tkns.rewind();
		return nullptr;
	}
	else if (first.type == TokenType::OPERATOR)
	{
		if (auto foundOp = UNARY_OPS.find(first.str); foundOp != UNARY_OPS.end())
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

				auto const& absEnd = tkns.cur();

				if (absEnd.str != "|")
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

sptr<Expr> Parser::parseParenValue()
{
	size_t const start = tkns.offset();

	auto const first = tkns.cur();

	if (first.type != TokenType::START_PAREN)
	{
		return nullptr;
	}

	tkns.consume();

	auto v = parseExpr();

	if (v == nullptr)
	{
		tkns.revertTo(start);
		return nullptr;
	}

	//TODO tuple literals go here

	auto const& last = tkns.cur();

	if (last.type == TokenType::END_PAREN)
	{
		tkns.consume();
	}
	else
	{
		tkns.revertTo(start);
		return nullptr;
	}

	return v;
}

sptr<Expr> Parser::parseAccess()
{
	if (tkns.cur().type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	sptr<Expr> target = new_sptr<VarReadValue>(tkns.cur());

	tkns.consume();

	return target;
}

sptr<Expr> Parser::parseMemberAccess(sptr<Expr> target)
{
	auto chain = new_sptr<MemberReadChainValue>(target);

	while (tkns.hasRem(2))
	{
		if (tkns.cur().type != TokenType::PERIOD)
		{
			break;
		}

		auto const name = tkns.peek(1);

		if (name.type != TokenType::IDENTIFIER)
		{
			break;
		}

		//consume the period
		tkns.consume();

		if (tkns.hasRem(3))
		{
			auto& fnStart = tkns.peek(1);

			if (fnStart.type == TokenType::START_PAREN || fnStart.str == GENERIC_START)
			{
				if (auto mCall = parseMethodCall(chain))
				{
					return mCall;
				}

			}

		}

		chain->mems.push_back(name);

		tkns.consume();

	}

	if (chain->mems.empty())
	{
		return target;
	}

	return chain;
}

sptr<Expr> Parser::parseAnyFnCall()
{
	return parseFnCall(parseAccess());
}

sptr<Expr> Parser::parseFnCall(sptr<Expr> name)
{
	if (name == nullptr)
	{
		return nullptr;
	}

	if (!tkns.hasRem(2))
	{
		return nullptr;
	}

	size_t const tknStart = tkns.offset();

	auto fnCall = new_sptr<FnCallValue>(name);

	if (tkns.cur().str == GENERIC_START)
	{
		if (!(fnCall->genArgs = parseGenericArgs()))
		{
			tkns.revertTo(tknStart);
			return nullptr;
		}

	}

	if (!parseAnyBetween("(", LAMBDA() {
		fnCall->args = parseValueList();
	}, ")"))
	{
		tkns.revertTo(tknStart);
		return nullptr;
	}

	//catch that ending parentheses
	fnCall->end = tkns.peekBack(1);

	return fnCall;
}

sptr<Expr> Parser::parseMethodCall(sptr<Expr> target)
{
	if (target == nullptr)
	{
		return nullptr;
	}

	size_t const tknStart = tkns.offset();

	if (!tkns.hasRem(3))
	{
		return nullptr;
	}

	auto const name = tkns.cur();

	if (name.type != TokenType::IDENTIFIER)
	{
		return nullptr;
	}

	tkns.consume();

	auto mCall = new_sptr<MethodCallValue>(target, name);

	if (tkns.cur().str == GENERIC_START)
	{
		if (!(mCall->genArgs = parseGenericArgs()))
		{
			tkns.revertTo(tknStart);
			return nullptr;
		}

	}

	if (!parseAnyBetween("(", LAMBDA() {
		mCall->args = parseValueList();
	}, ")"))
	{
		tkns.revertTo(tknStart);
		return nullptr;
	}

	//catch that ending parentheses
	mCall->end = tkns.peekBack(1);

	return mCall;
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
	auto const first = tkns.cur();
	sptr<ParsedType> typeHint = nullptr;

	if (first.type != TokenType::KEYWORD)
	{
		return nullptr;
	}

	if (first.str != "const")
	{
		return nullptr;
	}

	if (tkns.cur().str == ":")
	{
		typeHint = parseTypeName();
	}

	auto const& name = tkns.cur();
	
	if (name.type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid global variable name", name);
		return nullptr;
	}

	auto v = new_sptr<GlobalVariable>(name.str);

	v->first = first;
	v->nameTkn = name;
	v->typeHint = typeHint;

	auto const& eqSign = tkns.next();

	if (eqSign.type != TokenType::SETTER)
	{
		auto e = errors->err("Global variables must be initialized", eqSign);

		e->note("Add an equal sign and a value");

		return v;
	}

	tkns.consume();

	if (auto init = parseExpr())
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

sptr<ParsedVar> Parser::parseMemberVar()
{
	auto mods = parseStmtMods();

	auto const start = tkns.cur();
	bool isConst = false;
	
	if (start.type == TokenType::KEYWORD && start.str == "const")
	{
		isConst = true;
		tkns.consume();
	}

	auto const typeStart = tkns.cur();
	auto const type = parseTypeName();

	if (type == nullptr)
	{
		auto e = errors->err("Invalid member variable type name starts here", typeStart);

		return nullptr;
	}

	auto const name = tkns.cur();
	
	if (name.type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Invalid member variable name", name);

		return nullptr;
	}

	tkns.consume();

	sptr<Expr> initValue = nullptr;

	if (tkns.cur().type == TokenType::SETTER)
	{
		tkns.consume();

		initValue = parseExpr();
	}

	return new_sptr<ParsedVar>(ParsedVar{
		mods, start, isConst, type, name, initValue
	});
}

std::vector<sptr<ParsedVar>> Parser::parseLocalVarLike()
{
	std::vector<sptr<ParsedVar>> vars;
	auto const first = tkns.cur();

	if (first.type != TokenType::KEYWORD)
	{
		return vars;
	}

	bool isConst = false;

	if (first.str == "const")
	{
		isConst = true;
	}
	else if (first.str != "var")
	{
		return vars;
	}

	tkns.consume();

	sptr<ParsedType> typeHint = nullptr;

	if (tkns.cur().str == ":")
	{
		auto const typeStart = tkns.next();

		if (auto const t = parseTypeName())
		{
			typeHint = t;
		}
		else
		{
			auto e = errors->err("Invalid type name", typeStart);
			return vars;
		}

	}

	auto const varNames = parseIdentifierList();

	if (varNames.empty())
	{
		auto e = errors->err("Invalid local variable name", tkns.cur());
		
		return vars;
	}

	tkns.consume();

	if (tkns.cur().str == ":")
	{
		auto e = errors->err("That's not how type hints work in Caliburn. Type hints go before the variable name, not after. So var: int x is valid. var x: int isn't.", tkns.cur());

		return vars;
	}

	sptr<Expr> initValue = nullptr;

	if (tkns.cur().type == TokenType::SETTER)
	{
		auto const valStart = tkns.next();

		if (auto v = parseExpr())
		{
			initValue = v;
		}
		else
		{
			auto e = errors->err("Invalid initializer value", valStart);
			
			return vars;
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
	auto const first = tkns.cur();

	if (first.type == TokenType::KEYWORD && first.str == "const")
	{
		isConst = true;
		tkns.consume();
	}

	auto const typeHint = parseTypeName();

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

	sptr<Expr> initValue = nullptr;

	if (tkns.cur().type == TokenType::SETTER)
	{
		auto const valStart = tkns.next();

		if (auto v = parseExpr())
		{
			initValue = v;

		}
		else
		{
			auto e = errors->err("Invalid member variable initial value", valStart);
			return vars;
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

std::vector<FnArg> Parser::parseFnArgs()
{
	std::vector<FnArg> fnArgs;

	while (tkns.hasCur())
	{
		auto argType = parseTypeName();
		auto name = tkns.cur();

		if (argType == nullptr)
		{
			break;
		}

		if (name.type != TokenType::IDENTIFIER)
		{
			errors->err("Not a valid argument name", name);
			break;
		}

		fnArgs.push_back(FnArg{ argType, name.str });

		if (tkns.next().type != TokenType::COMMA)
		{
			break;
		}

		tkns.consume();

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

	auto const first = tkns.cur();
	auto const name = tkns.peek(1);

	if (first.str != "@")
	{
		return nullptr;
	}

	if (name.type != TokenType::IDENTIFIER)
	{
		auto e = errors->err("Annotations must be named with an identifier", first, name);
		return nullptr;
	}

	tkns.consume(2);

	auto const startParen = tkns.cur();

	Token last;

	if (startParen.type != TokenType::START_PAREN)
	{
		last = name;
		return new_uptr<Annotation>(first, name, last);
	}

	tkns.consume();
	int closingParenNeeded = 1;

	std::vector<Token> contents;

	while (tkns.hasCur())
	{
		auto const& tkn = tkns.cur();

		if (tkn.type == TokenType::END_PAREN)
		{
			--closingParenNeeded;

			if (closingParenNeeded == 0)
			{
				break;
			}

		}
		else if (tkn.type == TokenType::START_PAREN)
		{
			++closingParenNeeded;
		}

		contents.push_back(tkn);
		tkns.consume();

	}

	if (closingParenNeeded != 0)
	{
		auto e = errors->err("Ran out of symbols and could not close annotation", startParen);
	}

	last = tkns.cur();

	tkns.consume();

	return new_uptr<Annotation>(first, name, contents, last);
}
