
#include "types/typestruct.h"

using namespace caliburn;

cllr::SSA RealStruct::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
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
