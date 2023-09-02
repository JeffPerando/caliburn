
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
	struct BaseType;
	struct RealType;
	
	using Symbol = std::variant<std::monostate, sptr<Module>, sptr<Function>, sptr<Value>, sptr<Variable>, sptr<BaseType>, sptr<RealType>>;

	struct SymbolTable
	{
	private:
		HashMap<std::string, Symbol> symbols;
		sptr<SymbolTable> parent;

	public:
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

			return Symbol();
		}

		bool isChildOf(sptr<SymbolTable> table) const
		{
			if (table == nullptr)
			{
				return false;
			}

			if (parent == nullptr)
			{
				return table.get() == this;
			}

			if (table == parent)
			{
				return true;
			}

			return parent->isChildOf(table);
		}

		//TODO does not seem to work
		void reparent(sptr<SymbolTable> p)
		{
			//parent = p;
		}

	};

}
