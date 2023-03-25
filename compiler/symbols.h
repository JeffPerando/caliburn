
#pragma once

#include <map>

#include "langcore.h"

namespace caliburn
{
	enum class SymbolType
	{
		UNKNOWN,
		MODULE,
		FUNCTION,
		VARIABLE,
		TYPE
	};

	struct Symbol
	{
		const SymbolType type;
		const void* data;

	};

	class SymbolTable
	{
		SymbolTable* const parent;
		std::map<std::string, Symbol*> symbols;

		SymbolTable() : parent(nullptr) {}
		SymbolTable(SymbolTable* p) : parent(p) {}
		virtual ~SymbolTable() {}
	public:
		bool add(std::string name, SymbolType type, void* data)
		{
			auto sym = find(name);

			if (sym)
			{
				return false;
			}

			symbols.emplace(name, new Symbol{ type, data });

			return true;
		}

		Symbol* find(std::string name) const
		{
			auto result = symbols.find(name);

			if (result != symbols.end())
			{
				return result->second;
			}
			
			if (parent != nullptr)
			{
				return parent->find(name);
			}

			return nullptr;
		}

	};

}
