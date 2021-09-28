
#pragma once

#include <map>
#include <vector>

#include "type.h"
#include "syntax.h"

namespace caliburn
{
	enum class StorageModifier
	{
		NONE,
		PUBLIC,
		PROTECTED,
		PRIVATE,
		SHARED,
		STATIC

	};

	StorageModifier parseStorageMod(std::string str);

	enum class SymbolType
	{
		NONE, TYPE, VARIABLE, FUNCTION
	};

	//Symbol is defined along with statements; this is because symbols contain statements.
	//the usage here is limited to ProgramContext
	struct Symbol;

	struct FunctionSignature
	{
		std::string name;
		//only used for methods
		CompiledType* memberType = nullptr;
		CompiledType* returnType;
		std::vector<CompiledType*> args;

		bool isCompatible(FunctionSignature* rhs) const
		{
			if (name != rhs->name)
			{
				return false;
			}

			if (memberType != rhs->memberType)
			{
				if (memberType == nullptr || rhs->memberType == nullptr)
				{
					return false;
				}

				if (*memberType != *rhs->memberType)
				{
					return false;
				}

			}
			
			if (*returnType != *rhs->returnType)
			{
				return false;
			}

			if (args.size() != rhs->args.size())
			{
				return false;
			}

			for (size_t i = 0; i < args.size(); ++i)
			{
				if (*args[i] != *rhs->args[i])
				{
					return false;
				}

			}

			return true;
		}

	};

	struct FunctionData
	{
		FunctionSignature sig;
		std::vector<StorageModifier> mods;

	};

	struct FieldData
	{
		std::string name;
		CompiledType* type;
		std::vector<StorageModifier> mods;

	};

	struct ProgramContext
	{
		std::map<std::string, Symbol*> symbols;
		
	};

}
