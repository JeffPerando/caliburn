
#include "types/typestruct.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

sptr<cllr::LowType> TypeStruct::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (auto found = variants.find(gArgs); found != variants.end())
	{
		return found->second;
	}

	auto memberTable = new_sptr<SymbolTable>(table);

	//populate table with generics and members
	gArgs->apply(*genSig, memberTable, codeAsm);

	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_STRUCT, { (uint32_t)members.size() }));

	for (auto& [name, type] : members)
	{
		if (auto rt = type->typeHint->resolve(memberTable, codeAsm))
		{
			impl->addMember(name, rt);
			
		}
		else
		{
			//TODO complain
		}

	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::STRUCT_END, {}, { impl->id }));

	//avoid allocating the parsed type
	if (memberFns.empty())
	{
		return impl;
	}

	auto me = new_sptr<ParsedType>(canonName, gArgs);

	for (auto& fn : memberFns)
	{
		impl->addMemberFn(new_sptr<SrcMethod>(me, *fn));

	}

	return impl;
}
