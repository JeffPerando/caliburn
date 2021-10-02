
#pragma once

#include <map>
#include <vector>

#include "syntax.h"

namespace caliburn
{
	auto static constexpr MIN_INT_BITS_SUPPORTED = 8;
	auto static constexpr MAX_INT_BITS_SUPPORTED = 512;
	
	auto static constexpr MIN_FLOAT_BITS_SUPPORTED = 16;
	auto static constexpr MAX_FLOAT_BITS_SUPPORTED = 64;

	auto static constexpr MIN_VECTOR_LEN_SUPPORTED = 2;
	auto static constexpr MAX_VECTOR_LEN_SUPPORTED = 4;

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

	struct FunctionArg
	{
		std::string name;
		CompiledType* type;
		//ValueStatement* defaultValue = nullptr;

		//operators have been moved to langcore.cpp
	};

	struct FunctionSignature
	{
		std::string name;
		//only used for methods
		CompiledType* memberType = nullptr;
		CompiledType* returnType = nullptr;
		std::vector<FunctionArg*> args;

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
		FieldData data;

		VarSymbol()
		{
			symbolType = SymbolType::VARIABLE;
		}

	};

	struct TypeSymbol : public Symbol
	{
		CompiledType* type = nullptr;

		TypeSymbol()
		{
			symbolType = SymbolType::TYPE;
		}

	};

	struct FunctionSymbol : public Symbol
	{
		std::map<FunctionSignature, FunctionData*> functions;

		FunctionSymbol()
		{
			symbolType = SymbolType::FUNCTION;
		}

	};

	struct SymbolTable
	{
		std::vector<Symbol*> symList;
		std::map<std::string, Symbol*> symMap;

		SymbolTable() {}

		~SymbolTable()
		{
			for (auto sym : symList)
			{
				delete sym;
			}

		}

		virtual bool add(Symbol* sym);
		virtual Symbol* resolve(std::string name);

	};

	enum class TypeConvertCompat
	{
		COMPATIBLE,
		NEEDS_CONVERSION,
		INCOMPATIBLE
	};

	struct TypeConvertResult
	{
		TypeConvertCompat compat;
		CompiledType* commonType;

	};

}
