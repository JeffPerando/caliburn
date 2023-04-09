
#include "ast.h"
#include "type.h"

using namespace caliburn;

ConcreteType* ParsedType::resolve(ref<const SymbolTable> table)
{
	auto cTypeSym = table.find(name->str);

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

	auto cType = (ConcreteType*)cTypeSym->data;

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

	auto clone = cType->clone();

	for (size_t i = 0; i < generics.size(); ++i)
	{
		auto gType = generics.at(i);

		auto gConcrete = gType->resolve(table);

		if (gConcrete == nullptr)
		{
			//Might have the potential to try and delete pointers it shouldn't.
			delete clone;
			return nullptr;
		}

		clone->setGeneric(i, gConcrete);

	}

	resultType = clone;
	return clone;
}
/*
void Variable::getSSAs(cllr::Assembler& codeAsm)
{
	id = codeAsm.createSSA(cllr::Opcode::VAR_LOCAL);

}
*/
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
	codeAsm.push(id, cllr::Opcode::VAR_LOCAL, {}, { type->id, value->id, 0 });

}
