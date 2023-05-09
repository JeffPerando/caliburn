
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
		const sptr<void> data;

		Symbol(SymbolType t, sptr<void> d) : type(t), data(d) {}

	};

	class SymbolTable
	{
		std::map<std::string, sptr<Symbol>> symbols;
		
	public:
		const sptr<const SymbolTable> parent;

		SymbolTable() : parent(nullptr) {}
		SymbolTable(sptr<SymbolTable> p) : parent(p) {}
		virtual ~SymbolTable() {}

		bool add(std::string symName, SymbolType type, sptr<void> data)
		{
			auto sym = find(symName);

			if (sym)
			{
				return false;
			}

			symbols.emplace(symName, std::make_shared<Symbol>(type, data));

			return true;
		}

		sptr<Symbol> find(std::string symName) const
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
