
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
		VALUE,
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
		std::map<std::string, Symbol*> symbols;
		
	public:
		const ptr<const SymbolTable> parent;

		SymbolTable() : parent(nullptr) {}
		SymbolTable(SymbolTable* p) : parent(p) {}
		virtual ~SymbolTable()
		{
			for (auto& table : symbols)
			{
				delete table.second;
			}

			symbols.clear();

		}

		bool add(std::string symName, SymbolType type, void* data)
		{
			auto sym = find(symName);

			if (sym)
			{
				return false;
			}

			symbols.emplace(symName, new Symbol{ type, data });

			return true;
		}

		Symbol* find(std::string symName) const
		{
			auto result = symbols.find(symName);

			if (result != symbols.end())
			{
				return result->second;
			}
			
			if (parent != nullptr)
			{
				return parent->find(symName);
			}

			return nullptr;
		}

	};

}
