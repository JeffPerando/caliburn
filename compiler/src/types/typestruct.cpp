
#include "types/typestruct.h"

#include "cllr/cllrtype.h"

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
	std::vector<std::pair<std::string, sptr<FunctionGroup>>> memberFns;

	for (auto& [name, mem] : parent->members)
	{
		if (auto v = std::get_if<sptr<Variable>>(&mem))
		{
			memberVars.push_back(*v);

		}
		else if (auto fn = std::get_if<sptr<FunctionGroup>>(&mem))
		{
			memberFns.push_back(std::pair(name, *fn));
		}

	}

	auto [typeID, typeImpl] = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_STRUCT, { (uint32_t)memberVars.size() }));

	id = typeID;

	for (auto const& v : memberVars)
	{
		v->emitDeclCLLR(memberTable, codeAsm);
	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::STRUCT_END, {}, { id }));

	for (auto& [name, fn] : memberFns)
	{
		typeImpl->setMemberFns(name, fn);
	}

	return id;
}
