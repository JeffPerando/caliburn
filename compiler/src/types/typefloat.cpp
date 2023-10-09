
#include "types/typefloat.h"

using namespace caliburn;

sptr<cllr::LowType> RealFloat::emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (impl == nullptr)
	{
		auto p = (ptr<TypeFloat>)base;

		impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_FLOAT, { p->width }));

	}

	return impl;
}
