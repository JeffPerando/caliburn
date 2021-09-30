
#include "assembler.h"

using namespace caliburn;

CompiledType* CaliburnAssembler::resolveType(ParsedType* type)
{
	auto it = defaultTypes.find(type->getBasicName());
	CompiledType* resolved;

	if (it == defaultTypes.end())
	{
		//TODO find custom type
		resolved = nullptr;

	}
	else
	{
		resolved = it->second;

	}

	if (!resolved)
	{
		return nullptr;
	}

	if (resolved->hasA(TypeAttrib::GENERIC))
	{
		if (type->generics.size() < ((SpecializedType*)resolved)->minGenerics ||
			type->generics.size() > ((SpecializedType*)resolved)->maxGenerics)
		{
			//TODO complain
			return nullptr;
		}

		if (type->getFullName() == resolved->getFullName())
		{
			return resolved;
		}

		//gonna make a new generic type
		resolved = resolved->clone();

		for (auto i = 0; i < type->generics.size(); ++i)
		{
			CompiledType* compiledGeneric = resolveType(type->generics[i]);
			resolved->setGeneric(i, compiledGeneric);

		}

	}
	else if (type->generics.size() > 0)
	{
		//TODO complain about inappropriate generic
		return nullptr;
	}

	return resolved;
}