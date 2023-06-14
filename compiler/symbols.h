
#pragma once

#include <map>
#include <variant>

#include "langcore.h"

namespace caliburn
{
	struct Module;
	struct Function;
	struct Value;
	struct Variable;
	struct Type;

	using Symbol = std::variant<nullptr_t, sptr<Module>, sptr<Function>, sptr<Value>, sptr<Variable>, sptr<Type>>;

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
