
#include "cllr/cllrtype.h"
#include "types/typevector.h"

using namespace caliburn;

sptr<cllr::LowType> TypeVector::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (!genSig->canApply(*gArgs))
	{
		//TODO complain
		return nullptr;
	}

	if (gArgs->empty())
	{
		gArgs = genSig->makeDefaultArgs();
	}

	if (auto found = variants.find(gArgs); found != variants.end())
	{
		return found->second;
	}

	auto inner = gArgs->getType(0)->resolve(table, codeAsm);
	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_VECTOR, { elements }, { inner->id }));

	variants.insert(std::pair(gArgs, impl));

	return impl;
}
