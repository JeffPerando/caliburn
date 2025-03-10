
#include "ast/ast.h"
#include "ast/generics.h"
#include "ast/type.h"

#include "cllr/cllrasm.h"

using namespace caliburn;

void GenericArguments::apply(in<GenericSignature> sig, sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	for (size_t i = 0; i < args.size(); ++i)
	{
		auto const& name = sig.names[i].name;
		auto const& arg = args[i];

		if (auto vArg = std::get_if<sptr<Expr>>(&arg))
		{
			table->add(name, *vArg);
		}
		else if (auto tArg = std::get_if<sptr<ParsedType>>(&arg))
		{
			if (auto t = (**tArg).resolve(table, codeAsm))
			{
				table->add(name, t);
			}
			
			//TODO complain
		}
		else
		{
			//TODO complain
		}

	}

}

void GenericArguments::prettyPrint(out<std::stringstream> ss) const
{
	if (empty())
	{
		return;
	}

	ss << '<';

	for (size_t i = 0; i < args.size(); ++i)
	{
		auto const& arg = args[i];

		ss << parseGeneric(arg);

		if (i + 1 < args.size())
		{
			ss << ", ";

		}

	}

	ss << '>';

}

void GenericSignature::prettyPrint(out<std::stringstream> ss) const
{
	if (names.size() == 0)
	{
		return;
	}

	ss << '<';

	for (size_t i = 0; i < names.size(); ++i)
	{
		auto const& name = names[i];

		ss << GENERIC_TYPE_NAMES.at(name.type) << ' ' << name.name;

		if (!std::holds_alternative<std::monostate>(name.defaultResult))
		{
			ss << " = " << parseGeneric(name.defaultResult);
		}

		if (i + 1 < names.size())
		{
			ss << ", ";

		}

	}

	ss << '>';

}

bool GenericSignature::canApply(in<GenericArguments> genArgs) const
{
	auto const& args = genArgs.args;

	if (args.size() > names.size())
	{
		return false;
	}

	if (args.size() < minArgs)
	{
		return false;
	}

	bool valid = true;

	for (size_t i = 0; i < args.size(); ++i)
	{
		auto const& arg = args[i];
		auto const& name = names[i];

		if (auto vArg = std::get_if<sptr<Expr>>(&arg))
		{
			if (name.type != GenericSymType::CONST)
			{
				valid = false;
				//TODO complain
			}

			if (!(**vArg).isCompileTimeConst())
			{
				//TODO complain
			}

		}
		else if (auto tArg = std::get_if<sptr<ParsedType>>(&arg))
		{
			if (name.type != GenericSymType::TYPE)
			{
				valid = false;
				//TODO complain
			}

		}
		else
		{
			valid = false;
		}

	}

	return valid;
}

std::string caliburn::parseGeneric(in<GenericResult> result)
{
	if (auto vArg = std::get_if<sptr<Expr>>(&result))
	{
		return (**vArg).prettyStr();
	}
	else if (auto tArg = std::get_if<sptr<ParsedType>>(&result))
	{
		return (**tArg).prettyStr();
	}

	//TODO complain
	return "";
}

size_t GenericArgHash::operator()(sptr<GenericArguments> args) const
{
	size_t hash = 0;

	for (auto const& result : args->args)
	{
		hash ^= std::hash<std::string>{}(parseGeneric(result));
	}

	return hash;
}
