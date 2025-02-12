
#include "ast/symbols.h"

#include "ast/basetypes.h"

using namespace caliburn;

void SymbolTable::reparent(sptr<const SymbolTable> p)
{
    if (this == nullptr)
	{
        throw std::invalid_argument("You forgot to initialize your symbol table, goofus");
    }

    parent = p;
}

bool SymbolTable::add(std::string_view symName, in<Symbol> sym)
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

Symbol SymbolTable::find(std::string_view symName) const
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

bool SymbolTable::has(std::string_view symName) const
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
