
#include "types/type.h"
#include "ast/generics.h"

using namespace caliburn;

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

		ss << name.type->str << ' ' << name.name->str;

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

bool GenericSignature::canApply(ref<GenericArguments> genArgs)
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

		//TODO long for the days of pattern matching

		auto vArg = std::get_if<uptr<Value>>(&arg);

		if (vArg != nullptr)
		{
			if (name.type->str != "const")
			{
				//TODO complain
			}

			continue;
		}

		auto tArg = std::get_if<uptr<ParsedType>>(&arg);

		if (tArg != nullptr)
		{
			if (name.type->str != "type")
			{
				//TODO complain
			}

			continue;
		}

		return false;
	}

	return true;
}

std::string caliburn::parseGeneric(ref<const GenericResult> result)
{
	auto vArg = std::get_if<uptr<Value>>(&result);

	if (vArg != nullptr)
	{
		return (*vArg)->prettyStr();
	}

	auto tArg = std::get_if<uptr<ParsedType>>(&result);

	if (tArg != nullptr)
	{
		return (*tArg)->prettyStr();
	}

	//TODO complain
	return "";
}
