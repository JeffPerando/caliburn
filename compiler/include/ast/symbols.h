
#pragma once

#include <map>
#include <variant>

#include "langcore.h"

namespace caliburn
{
	namespace cllr
	{
		struct LowType;
	}

	struct Module;
	struct Function;
	struct Value;
	struct Variable;
	struct BaseType;
	
	/*
	All symbol variants are shared because nothing really owns them. No, not the symbol table, since the table is just
	giving you access to an object somewhere else in memory, created before it was added here.

	Would it make more sense to just work with sptr<Symbol> and have the variants be direct structs? Yes, but it would be
	more expensive in memory. Variants are always the size of their biggest version, even in Rust. So this way, we keep our
	memory footprint down, at the expense of a little (debatable) code wonkiness.
	*/
	using Symbol = std::variant<std::monostate,
		sptr<Module>,
		sptr<Function>,
		//this enables for generic constants
		sptr<Value>,
		sptr<Variable>,
		sptr<BaseType>,
		//This is only used when working with generics
		sptr<cllr::LowType>>;

	/*
	A symbol table is just that: a way to maintain a directory between names and objects.

	Since having one massive symbol table would be a pain to maintain, we use a parenting strategy.
	Yes, this is a glorified linked list, but that's fine.
	*/
	struct SymbolTable
	{
	private:
		HashMap<std::string, Symbol> symbols;
		sptr<const SymbolTable> parent;

	public:
		SymbolTable() : parent(nullptr) {}
		SymbolTable(sptr<const SymbolTable> p) : parent(p) {}
		virtual ~SymbolTable() {}

		bool add(in<std::string> symName, in<Symbol> sym)
		{
			if (symbols.find(symName) != symbols.end())
			{
				return false;
			}

			symbols.emplace(symName, sym);

			return true;
		}

		Symbol find(in<std::string> symName) const
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

		bool has(in<std::string> symName) const
		{
			return !std::holds_alternative<std::monostate>(find(symName));
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
		void reparent(sptr<const SymbolTable> p)
		{
			//parent = p;
		}

	};

}
