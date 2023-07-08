/*
#include "types/typestruct.h"

using namespace caliburn;

uint32_t TypeStruct::getSizeBytes() const
{
	
	uint32_t size = 0;
	uint32_t lastAlign = 1;

	for (auto const& m : members)
	{
		//auto t = m->typeHint->resolve
		size += t->getSizeBytes();

		auto align = t->getAlignBytes();

		//calculate padding
		if (size % align != lastAlign)
		{
			size += ((size / lastAlign) + 1) * lastAlign;
		}

		lastAlign = align;

	}

	return size;
}

uint32_t TypeStruct::getAlignBytes() const
{
	if (members.empty())
	{
		return 1;
	}
	//return members.back()->realType->getAlignBytes();
}

sptr<Type> TypeStruct::makeVariant(ref<std::vector<sptr<Type>>> genArgs) const override
{
	auto ret = new_uptr<TypeStruct>(canonName, maxGenerics);

	//TODO check compatibility

	for (size_t i = 0; i < genArgs.size(); ++i)
	{
		ret->setGeneric(i, genArgs.at(i));
	}

	return ret;
}

TypeCompat TypeStruct::isCompatible(Operator op, sptr<BaseType> rType) const
{
	return TypeCompat::INCOMPATIBLE_TYPE;
}

cllr::SSA TypeStruct::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	auto sID = codeAsm.pushNew(cllr::Opcode::TYPE_STRUCT, {}, {});

	for (sptr<Variable> m : members)
	{
		m->emitDeclCLLR(table, codeAsm);

	}

	return sID;
}
*/