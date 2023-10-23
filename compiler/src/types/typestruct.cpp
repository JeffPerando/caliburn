
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

	std::vector<sptr<Variable>> memberVars;
	std::vector<std::pair<std::string, sptr<FunctionGroup>>> memberFns;

	for (auto& [name, mem] : members)
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

	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_STRUCT, { (uint32_t)memberVars.size() }));

	for (auto const& v : memberVars)
	{
		v->emitVarCLLR(memberTable, codeAsm);
	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::STRUCT_END, {}, { impl->id }));

	for (auto& [name, fn] : memberFns)
	{
		impl->setMemberFns(name, fn);
	}

	return impl;
}
