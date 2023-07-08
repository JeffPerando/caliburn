
#pragma once

#include <map>
#include <variant>

#include "langcore.h"

namespace caliburn
{
	struct Module;
	class Function;
	struct Value;
	class Variable;
	class BaseType;
	
	using Symbol = std::variant<nullptr_t, sptr<Module>, sptr<Function>, sptr<Value>, sptr<Variable>, sptr<BaseType>>;

	class SymbolTable
	{
		HashMap<std::string, Symbol> symbols;
		
	public:
		const sptr<const SymbolTable> parent;

		SymbolTable() : parent(nullptr) {}
		SymbolTable(sptr<SymbolTable> p) : parent(p) {}
		virtual ~SymbolTable() {}

		bool add(std::string symName, Symbol sym)
		{
			if (symbols.find(symName) != symbols.end())
			{
				return false;
			}

			symbols.emplace(symName, sym);

			return true;
		}

		Symbol find(std::string symName) const
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
