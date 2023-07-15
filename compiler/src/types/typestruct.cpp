
#include "types/typestruct.h"

using namespace caliburn;

cllr::SSA RealStruct::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return id;
	}
	
	auto const& parent = static_cast<ptr<TypeStruct>>(base);

	auto memberTable = new_sptr<SymbolTable>(table);

	//populate table with generics and members
	genArgs->apply(parent->genSig, memberTable);

	std::vector<sptr<Variable>> memberVars;

	for (auto const& [name, mem] : parent->members)
	{
		if (auto v = std::get_if<sptr<Variable>>(&mem))
		{
			memberVars.push_back(*v);

		}

	}

	id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::TYPE_STRUCT, { (uint32_t)memberVars.size() }));

	for (auto const& v : memberVars)
	{
		v->emitDeclCLLR(memberTable, codeAsm);
	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::STRUCT_END, {}, { id }));

	return id;
}


/*

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