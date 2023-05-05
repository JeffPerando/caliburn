
#include "ast.h"
#include "type.h"

using namespace caliburn;

ptr<Type> ParsedType::resolve(ref<const SymbolTable> table)
{
	auto lookup = &table;

	if (mod != nullptr)
	{
		auto modSym = table.find(mod->str);

		if (modSym->type == SymbolType::MODULE)
		{
			lookup = (ptr<const SymbolTable>)modSym->data;
		}
		else
		{
			//TODO complain
			return nullptr;
		}

	}

	auto cTypeSym = lookup->find(name->str);

	if (cTypeSym == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	if (cTypeSym->type != SymbolType::TYPE)
	{
		//TODO complain
		return nullptr;
	}

	auto cType = (Type*)cTypeSym->data;

	if (cType->maxGenerics == 0 && this->generics.size() == 0)
	{
		resultType = cType;
		return cType;
	}

	if (cType->maxGenerics > this->generics.size())
	{
		//TODO complain
		return nullptr;
	}

	if (cType->minGenerics < this->generics.size())
	{
		//TODO complain
		return nullptr;
	}

	try
	{
		std::vector<ptr<Type>> resolvedGenerics;

		for (size_t i = 0; i < generics.size(); ++i)
		{
			auto gType = generics.at(i);

			auto gConcrete = gType->resolve(table);

			if (gConcrete == nullptr)
			{
				//TODO complain
			}

			resolvedGenerics.push_back(gConcrete);

		}

		resultType = cType->makeVariant(resolvedGenerics);

	}
	catch (std::exception e)
	{
		//TODO complain
	}
	
	return resultType;
}

void Variable::resolveSymbols(ref<const SymbolTable> table)
{
	if (typeHint != nullptr)
	{
		type = typeHint->resolve(table);

	}

}

void Variable::emitDeclCLLR(ref<cllr::Assembler> codeAsm)
{
	Value* value = initValue;

	if (type == nullptr)
	{
		type = initValue->type;
	}/*
	else if (value == nullptr)
	{
		value = type->defaultInitValue();
		value->emitValueCLLR(codeAsm);
	}
	*/
	auto vID = value->emitValueCLLR(codeAsm);

	id = codeAsm.pushNew(cllr::Opcode::VAR_LOCAL, {}, { type->id, vID, 0 });

}
