
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

	struct FunctionSignature
	{
		std::string name;
		//only used for methods
		CompiledType* memberType = nullptr;
		CompiledType* returnType = nullptr;
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
		Statement* code = nullptr;
		std::vector<StorageModifier> mods;

	};

	struct FieldData
	{
		std::string name;
		CompiledType* type = nullptr;
		std::vector<StorageModifier> mods;

	};

	enum class SymbolType
	{
		NONE, TYPE, VARIABLE, FUNCTION
	};

	struct Symbol
	{
		std::string name = "BROKEN SYMBOL PLS FIX";
		SymbolType symbolType = SymbolType::NONE;

	};

	struct VarSymbol : public Symbol
	{
		FieldData* data = nullptr;

	};

	struct TypeSymbol : public Symbol
	{
		CompiledType* type = nullptr;

	};

	struct FunctionSymbol : public Symbol
	{
		std::map<FunctionSignature, FunctionData*> functions;

	};

	struct ProgramContext
	{
		std::map<std::string, Symbol*> symbols;
		
	};

}
