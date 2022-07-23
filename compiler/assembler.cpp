
#include "assembler.h"

using namespace caliburn;

CaliburnAssembler::CaliburnAssembler()
{
	//int types
	for (int bits = MIN_INT_BITS;
		bits <= MAX_INT_BITS; bits *= 2)
	{
		auto intType = new TypeInt(bits, true);
		auto uintType = new TypeInt(bits, false);

		addDefaultType("int" + bits, intType);
		addDefaultType("uint" + bits, uintType);

		defaultIntTypes.push_back(intType);
		defaultIntTypes.push_back(uintType);

	}

	std::vector<std::string> vecPrefixes = { "h", "", "d" };

	//float types
	size_t vecPrefixOff = 0;
	for (int bits = MIN_FLOAT_BITS;
		bits <= MAX_FLOAT_BITS; bits *= 2)
	{
		auto floatType = new TypeFloat(bits);
		addDefaultType("float" + bits, floatType);
		defaultFloatTypes.push_back(floatType);

		if (vecPrefixOff >= vecPrefixes.size())
		{
			continue;
		}

		for (uint32_t e = MIN_VECTOR_LEN;
			e <= MAX_VECTOR_LEN; ++e)
		{
			//makes hvec, vec, and dvec types
			//can support qvec types if some weirdo wants 128-bit float support
			auto vecType = new TypeVector(e, floatType);
			addDefaultType(vecPrefixes[vecPrefixOff] + ("vec" + e), vecType);
			++vecPrefixOff;

			if (bits == 32)
			{
				defaultVectorTypes.push_back(vecType);

			}

		}

	}

	//int vector types

	CompiledType* si32 = getIntType(4);

	for (uint32_t e = MIN_VECTOR_LEN;
		e <= MAX_VECTOR_LEN; ++e)
	{
		addDefaultType("ivec" + e, new TypeVector(e, si32));

	}

	//void type
	addDefaultType("void", new TypeVoid());

	//TODO replace with proper character types that ensures chars get printed as chars
	addTypeAlias("char8", "uint8");
	addTypeAlias("char32", "uint32");

}

CaliburnAssembler::~CaliburnAssembler()
{
	for (auto type : defaultTypes)
	{
		delete type;

	}

	defaultTypes.clear();

}
/*
CompiledType* CaliburnAssembler::resolveType(ParsedType* type)
{
	SymbolTable* mod = &stdLib;
	
	if (type->mod && type->mod->str.length() > 0)
	{
		Symbol* modSym = stdLib.resolve(type->mod->str);

		if (!modSym || modSym->symbolType != SymbolType::MODULE)
		{
			//TODO bigger complaint
			return nullptr;
		}

		mod = ((ModuleSymbol*)modSym)->table;

		if (!mod)
		{
			//TODO bigger complaint
			return nullptr;
		}

	}

	auto typeSym = mod->resolve(type->getBasicName());

	if (!typeSym || typeSym->symbolType != SymbolType::TYPE)
	{
		//TODO bigger complaint
		return nullptr;
	}

	CompiledType* resolved = ((TypeSymbol*)typeSym)->type;

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
*/