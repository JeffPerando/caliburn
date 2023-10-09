
#include "cllr/cllrtype.h"
#include "types/typevector.h"

using namespace caliburn;

sptr<cllr::LowType> RealVector::emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (impl != nullptr)
	{
		return impl;
	}

	auto& t = genArgs->args[0];

	if (auto& ptype = std::get<sptr<ParsedType>>(t))
	{
		if (auto type = ptype->resolve(table))
		{
			auto innerImpl = type->emitTypeCLLR(table, codeAsm);

			impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_VECTOR, { length }, { innerImpl->id }));
			return impl;
		}

		//TODO complain

	}

	return impl;
}

Member TypeVector::getMember(in<std::string> name) const
{
	//TODO swizzle
	return Member();
}
