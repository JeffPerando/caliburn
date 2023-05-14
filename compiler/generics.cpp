
#include "type.h"
#include "generics.h"

using namespace caliburn;

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
		//TODO pretty print

	}

	auto tArg = std::get_if<uptr<ParsedType>>(&result);

	if (tArg != nullptr)
	{
		//TODO pretty print
	}

	//TODO complain
	return "";
}
