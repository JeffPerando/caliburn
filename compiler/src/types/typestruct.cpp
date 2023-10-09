
#include "types/typestruct.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

sptr<cllr::LowType> RealStruct::emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (impl != nullptr)
	{
		return impl;
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

	impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_STRUCT, { (uint32_t)memberVars.size() }));

	for (auto const& v : memberVars)
	{
		v->emitDeclCLLR(memberTable, codeAsm);
	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::STRUCT_END, {}, { impl->id }));

	for (auto& [name, fn] : memberFns)
	{
		impl->setMemberFns(name, fn);
	}

	return impl;
}
