
#include "type.h"

using namespace caliburn;

CompiledType* ParsedType::resolve(CaliburnAssembler* codeAsm, SymbolTable* syms)
{
	if (mod != nullptr && this->mod->str.length() > 0)
	{
		auto modSym = syms->resolve(this->mod->str);

		if (modSym == nullptr || modSym->symbolType != SymbolType::MODULE)
		{
			//TODO complain
			return nullptr;
		}

		syms = ((ModuleSymbol*)modSym)->table;

	}

	auto typeSym = syms->resolve(name->str);

	if (typeSym->symbolType != SymbolType::TYPE)
	{
		//TODO complain
		return nullptr;
	}

	auto type = ((TypeSymbol*)typeSym)->type;

	if (!type)
	{
		//TODO complain
		return nullptr;
	}

	if (generics.size() == 0 && type->maxGenerics == 0)
	{
		return type;
	}

	if (generics.size() > type->maxGenerics)
	{
		//TODO complain
		return nullptr;
	}

}