
#pragma once

#include <map>
#include <vector>

#include "syntax.h"

namespace caliburn
{
	struct Statement;
	struct CompiledType;

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

		bool operator==(const FunctionSignature& rhs) const;

	};

	struct FunctionData
	{
		FunctionSignature sig;
		Statement* code = nullptr;
		std::vector<StorageModifier> mods;

	};

	struct FieldData
	{
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
		VarSymbol()
		{
			symbolType = SymbolType::VARIABLE;
		}

		FieldData data;

	};

	struct TypeSymbol : public Symbol
	{
		TypeSymbol()
		{
			symbolType = SymbolType::TYPE;
		}

		CompiledType* type = nullptr;

	};

	struct FunctionSymbol : public Symbol
	{
		FunctionSymbol()
		{
			symbolType = SymbolType::FUNCTION;
		}

		std::map<FunctionSignature, FunctionData*> functions;

	};

	struct ProgramContext
	{
		std::map<std::string, Symbol*> symbols;
		
	};

}
