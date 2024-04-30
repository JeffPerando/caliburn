
#include "ast/symbols.h"

#include "types/type.h"

using namespace caliburn;

//TODO does not seem to work
void SymbolTable::reparent(sptr<const SymbolTable> p)
{
	//parent = p;
}

bool SymbolTable::add(in<std::string> symName, in<Symbol> sym)
{
	if (symbols.find(symName) != symbols.end())
	{
		return false;
	}

	symbols.emplace(symName, sym);

	return true;
}

bool SymbolTable::addType(sptr<BaseType> t)
{
	return add(t->canonName, t);
}

Symbol SymbolTable::find(in<std::string> symName) const
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

bool SymbolTable::has(in<std::string> symName) const
{
	return !std::holds_alternative<std::monostate>(find(symName));
}

bool SymbolTable::isChildOf(sptr<SymbolTable> table) const
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
