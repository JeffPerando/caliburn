
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
		TYPE
	};

	struct Symbol
	{
		const SymbolType type;
		const void* data;

	};

	class SymbolTable
	{
		const SymbolTable* parent;
		std::map<std::string, Symbol*> symbols;
		std::map<std::string, SymbolTable*> children;

	public:
		const std::string name;

		SymbolTable(std::string name = "") : parent(nullptr), name(name) {}
		SymbolTable(SymbolTable* p, std::string name = "") : parent(p), name(name) {}
		virtual ~SymbolTable()
		{
			for (auto& table : children)
			{
				delete table.second;
			}

			children.clear();

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

		SymbolTable* makeChild(std::string name = "", bool appendNum = false)
		{
			auto tbl = children.find(name);

			if (tbl != children.end())
			{
				if (!appendNum)
				{
					return tbl->second;
				}

				for (auto i = 1; i <= 1024; ++i)
				{
					auto newName = (std::stringstream() << name << '_' << i).str();

					tbl = children.find(newName);

					if (tbl == children.end())
					{
						name = newName;
						break;
					}

				}

			}

			auto table = new SymbolTable(this, name);

			children.emplace(name, table);

			return table;
		}

	};

}
