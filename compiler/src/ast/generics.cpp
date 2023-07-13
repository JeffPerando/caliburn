
#include <string>

#include "ast/generics.h"

#include "types/type.h"

using namespace caliburn;

void GenericArguments::apply(sptr<GenericSignature> sig, sptr<SymbolTable> table) const
{
	for (size_t i = 0; i < args.size(); ++i)
	{
		auto const& name = sig->names[i].name;
		auto const& arg = args[i];

		if (auto vArg = std::get_if<sptr<Value>>(&arg))
		{
			table->add(name, *vArg);
		}
		else if (auto tArg = std::get_if<sptr<ParsedType>>(&arg))
		{
			if (auto t = (**tArg).resolve(table))
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

void GenericArguments::prettyPrint(ref<std::stringstream> ss) const
{
	if (args.size() == 0)
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

void GenericSignature::prettyPrint(ref<std::stringstream> ss) const
{
	if (names.size() == 0)
	{
		return;
	}

	ss << '<';

	for (size_t i = 0; i < names.size(); ++i)
	{
		auto const& name = names[i];

		ss << name.typeTkn->str << ' ' << name.name;

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

bool GenericSignature::canApply(ref<GenericArguments> genArgs) const
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

	for (size_t i = 0; i < args.size(); ++i)
	{
		auto const& arg = args[i];
		auto const& name = names[i];

		if (auto vArg = std::get_if<sptr<Value>>(&arg))
		{
			if (name.type == GenericSymType::CONST)
			{
				continue;
			}

			//TODO complain

		}
		else if (auto tArg = std::get_if<sptr<ParsedType>>(&arg))
		{
			if (name.type == GenericSymType::TYPE)
			{
				continue;
			}

			//TODO complain
		}
		else return false;
	}

	return true;
}

/*
template<typename T>
sptr<T> caliburn::Generic<T>::makeVariant(sptr<GenericArguments> args)
{
	if (args == nullptr || args->empty())
	{
		args = sig->makeDefaultArgs();
	}

	if (!sig->canApply(*args))
	{
		//TODO complain
		return nullptr;
	}

	auto found = variants->find(args);

	if (found != variants->end())
	{
		return found->second;
	}

	auto newVar = new_sptr<T>(this, args);

	variants->emplace(args, newVar);

	return newVar;
}
*/
std::string caliburn::parseGeneric(ref<const GenericResult> result)
{
	if (auto vArg = std::get_if<sptr<Value>>(&result))
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
