
#include "types/type.h"

#include "ast/ast.h"

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

sptr<BaseType> ParsedType::resolveBase(sptr<const SymbolTable> table)
{
	auto typeSym = table->find(name);

	if (auto bType = std::get_if<sptr<BaseType>>(&typeSym))
	{
		return *bType;
	}

	return nullptr;
}

sptr<cllr::LowType> ParsedType::resolve(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (resultType != nullptr)
	{
		return resultType;
	}

	auto typeSym = table->find(name);

	if (auto lType = std::get_if<sptr<cllr::LowType>>(&typeSym))
	{
		if (!genericArgs->empty())
		{
			//TODO complain
		}

		resultType = *lType;
	}
	else if (auto bType = std::get_if<sptr<BaseType>>(&typeSym))
	{
		resultType = (**bType).resolve(genericArgs, table, codeAsm);
	}

	if (resultType == nullptr)
	{
		//TODO complain
	}

	return resultType;
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