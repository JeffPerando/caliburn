
#include "ast/type.h"

#include "ast/ast.h"
#include "ast/basetypes.h"

#include "parser.h"
#include "tokenizer.h"

using namespace caliburn;

void ParsedType::prettyPrint(out<std::stringstream> ss) const
{
	if (name == "")
	{
		ss << "INVALID TYPE PLS FIX";
		return;
	}

	if (fullName.length() == 0)
	{
		std::stringstream s;

		s << name;
		genericArgs->prettyPrint(s);

		s.str(fullName);

	}

	ss << fullName;

}

sptr<BaseType> ParsedType::resolveBase(sptr<const SymbolTable> table) const
{
	auto typeSym = table->find(name);

	if (auto bType = std::get_if<sptr<BaseType>>(&typeSym))
	{
		return *bType;
	}

	return nullptr;
}

sptr<cllr::LowType> ParsedType::resolve(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	if (this == nullptr)
	{
		throw new std::exception("you forgot to initialize your parsed type");
	}

	auto typeSym = table->find(name);

	MATCH_EMPTY(typeSym)
	{
		codeAsm.errors->err({ "Type not found:", name }, *this);
		return nullptr;
	}

	MATCH(typeSym, sptr<cllr::LowType>, lType)
	{
		if (!genericArgs->empty())
		{
			codeAsm.errors->err({ "Type not compatible with generic args:", name }, *this);
			return nullptr;
		}

		return *lType;
	}
	else MATCH(typeSym, sptr<BaseType>, bType)
	{
		return (**bType).resolve(genericArgs, table, codeAsm);
	}
	else
	{
		codeAsm.errors->err({ "Not a type:", name }, *this);
	}

	codeAsm.errors->err({ "Type resolution failed:", name }, * this);
	return nullptr;
}

sptr<ParsedType> ParsedType::parse(in<std::string> str)
{
	//Not a fan of doing this, BUT the code reuse is super easy
	Tokenizer t(new_sptr<TextDoc>(str));
	Parser p(t.tokenize());

	auto pt = p.parseTypeName();

	if (p.errors->empty())
	{
		//TODO do something with errors
		return nullptr;
	}

	if (p.hasTknsRem())
	{
		//TODO complain
		return nullptr;
	}

	return pt;
}