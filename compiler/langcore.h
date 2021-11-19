
#pragma once

#include <map>
#include <vector>

#include "syntax.h"

/*
Contains necessary constants, data types, and functions necessary for parsing, interpreting,
and compiling Caliburn code.

Also includes the syntax header in case you need that (I believe this does as well)
*/
namespace caliburn
{
	auto static constexpr MIN_INT_BITS = 8;
	auto static constexpr MAX_INT_BITS = 512;
	
	auto static constexpr MIN_FLOAT_BITS = 16;
	auto static constexpr MAX_FLOAT_BITS = 64;

	auto static constexpr MIN_VECTOR_LEN = 2;
	auto static constexpr MAX_VECTOR_LEN = 4;

	struct Statement;
	struct ParsedType;
	struct CompiledType;

	/*
	Enum denoting how much to optimize emitted shader code.

	Each level should emit more optimal code, obviously.
	
	Regarding whether to add a given algorithm to a given level: Consider how much extra performance is given vs. the time it takes to run.
	If the algorithm takes ages to run and doesn't make the code much faster, add it to O3.
	If it's near instant and makes the code much much faster, add it to O1.
	O0 always disables every optimization, even the obvious ones. This ensures clarity between the compiler and programmer. As in, the programmer can tell what the compiler is doing, based on the changes to code in O0 vs. others.
	*/
	enum class OptimizerLevel
	{
		//equivalent to O0; debug (none) level
		NONE,
		//equivalent to O1; just do low-hanging fruit
		SOME,
		//equivalent to O2; does more optimizations
		MOST,
		//equivalent to O3; does EVERY optimization
		ALL
	};

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
		CompiledType* memberOf = nullptr; //only used for methods
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
		ParsedType* pType = nullptr;
		CompiledType* cType = nullptr;
		std::vector<StorageModifier> mods;

	};

	enum class SymbolType
	{
		NONE, TYPE, VARIABLE, FUNCTION, MODULE
	};

	enum class VarType
	{
		//members are owned by a data type.
		MEMBER,
		//fields are owned by either a module or a function
		FIELD,
		//function args are arguments passed to a function. whoda thunk it?
		//note that this includes 'this'. Methods are compiled as functions with the first argument being the object used.
		FUNC_ARG
	};

	struct Symbol
	{
		std::string name = "";
		const SymbolType symbolType;

		Symbol(SymbolType t) : symbolType(t) {}

	};

	struct VarSymbol : public Symbol
	{
		FieldData data;
		const VarType varType;

		VarSymbol(VarType t) : Symbol(SymbolType::VARIABLE), varType(t) {}

	};

	struct TypeSymbol : public Symbol
	{
		CompiledType* type = nullptr;

		TypeSymbol() : Symbol(SymbolType::TYPE) {}

	};

	struct FunctionSymbol : public Symbol
	{
		std::map<FunctionSignature, FunctionData*> functions;

		FunctionSymbol() : Symbol(SymbolType::FUNCTION) {}

	};

	struct SymbolTable
	{
		SymbolTable* const parent;
		std::vector<Symbol*> symList;
		std::map<std::string, Symbol*> symMap;

		SymbolTable() : SymbolTable(nullptr) {}
		SymbolTable(SymbolTable* superTable) : parent(superTable) {}

		virtual ~SymbolTable()
		{
			for (auto sym : symList)
			{
				delete sym;
			}

		}

		virtual bool add(Symbol* sym);
		virtual bool alias(std::string name, Symbol* sym);
		virtual Symbol* resolve(std::string name);

	};

	struct ModuleSymbol : public Symbol
	{
		SymbolTable* table = nullptr;

		ModuleSymbol() : Symbol(SymbolType::MODULE) {}

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
