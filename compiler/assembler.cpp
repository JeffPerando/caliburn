
#include "assembler.h"

using namespace caliburn;

CaliburnAssembler::CaliburnAssembler()
{
	//int types
	for (int bits = MIN_INT_BITS_SUPPORTED;
		bits <= MAX_INT_BITS_SUPPORTED; bits *= 2)
	{
		auto intType = new TypeInt(bits, true);
		auto uintType = new TypeInt(bits, false);

		defaultTypes.emplace("int" + bits, intType);
		defaultTypes.emplace("uint" + bits, uintType);

		defaultIntTypes.push_back(intType);
		defaultIntTypes.push_back(uintType);

	}

	std::vector<std::string> vecPrefixes = { "h", "", "d" };

	//float types
	size_t vecPrefixOff = 0;
	for (int bits = MIN_FLOAT_BITS_SUPPORTED;
		bits <= MAX_FLOAT_BITS_SUPPORTED; bits *= 2)
	{
		auto floatType = new TypeFloat(bits);
		defaultTypes.emplace("float" + bits, floatType);
		defaultFloatTypes.push_back(floatType);

		if (vecPrefixOff >= vecPrefixes.size())
		{
			continue;
		}

		for (size_t e = MIN_VECTOR_LEN_SUPPORTED;
			e <= MAX_VECTOR_LEN_SUPPORTED; ++e)
		{
			//makes hvec, vec, and dvec types
			//can support qvec types if some weirdo wants 128-bit float support
			defaultTypes.emplace(vecPrefixes[vecPrefixOff] + ("vec" + e), new TypeVector(e, floatType));
			++vecPrefixOff;

		}

	}

	//int vector types

	CompiledType* si32 = getIntType(4);

	for (size_t e = MIN_VECTOR_LEN_SUPPORTED;
		e <= MAX_VECTOR_LEN_SUPPORTED; ++e)
	{
		defaultTypes.emplace("ivec" + e, new TypeVector(e, si32));

	}

	for (size_t e = MIN_VECTOR_LEN_SUPPORTED;
		e <= MAX_VECTOR_LEN_SUPPORTED; ++e)
	{
		defaultVectorTypes.push_back((TypeVector*)defaultTypes["vec" + e]);

	}

	//void type
	defaultVoidType = new TypeVoid();
	defaultTypes.emplace("void", defaultVoidType);

	//typical aliases
	addPermanentAlias("byte", "int8");
	//TODO replace with proper character types that ensures chars get printed as chars
	addPermanentAlias("char", "uint8");
	addPermanentAlias("char8", "uint8");
	addPermanentAlias("char32", "uint32");
	addPermanentAlias("short", "int16");
	addPermanentAlias("int", "int32");
	addPermanentAlias("long", "int64");
	addPermanentAlias("float", "float32");
	addPermanentAlias("double", "float64");

}

CaliburnAssembler::~CaliburnAssembler()
{
	for (auto type : defaultTypes)
	{
		delete type.second;

	}

}

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
		if (type->generics.size() > resolved->maxGenerics)
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