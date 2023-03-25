
#include "type.h"

using namespace caliburn;

ConcreteType* ParsedType::resolve(const SymbolTable& table)
{
	auto sym = table.find(name->str);

	if (sym->type != SymbolType::TYPE)
	{
		//TODO complain
		return nullptr;
	}

	auto cType = ((ConcreteType*)sym->data);

	if (cType->maxGenerics == 0 && this->generics.size() == 0)
	{
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

}

void Variable::getSSAs(cllr::Assembler& codeAsm)
{
	id = codeAsm.createSSA(cllr::Opcode::VAR_LOCAL);

}

void Variable::emitDeclCLLR(cllr::Assembler& codeAsm)
{
	ConcreteType* type = realType;
	Value* value = initValue;

	if (type == nullptr)
	{
		type = initValue->type;
	}
	else if (value == nullptr)
	{
		value = type->defaultInitValue();
		value->emitDeclCLLR(codeAsm);
	}

	codeAsm.push(id, cllr::Opcode::VAR_LOCAL, { type->id, value->id, 0 });

}
