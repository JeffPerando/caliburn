
#include "ast.h"
#include "type.h"

using namespace caliburn;

sptr<Type> ParsedType::resolve(sptr<const SymbolTable> table)
{
	auto cTypeSym = table.get()->find(name->str);
	auto cTypePtr = std::get_if<sptr<Type>>(&cTypeSym);

	if (cTypePtr == nullptr)
	{
		//TODO complain
		return nullptr;
	}

	auto const& cType = *cTypePtr;

	resultType = cType;

	//TODO replace
	/*
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
		std::vector<sptr<Type>> resolvedGenerics;

		for (size_t i = 0; i < generics.size(); ++i)
		{
			auto const& gType = generics.at(i);

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
	*/
	return resultType;
}

void Variable::resolveSymbols(sptr<const SymbolTable> table)
{
	if (typeHint != nullptr)
	{
		type = typeHint->resolve(table);

	}

}

void Variable::emitDeclCLLR(ref<cllr::Assembler> codeAsm)
{
	auto const& value = initValue;

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
