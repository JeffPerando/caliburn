
#include "types/typeint.h"

using namespace caliburn;

sptr<cllr::LowType> RealInt::emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (impl == 0)
	{
		auto p = (ptr<TypeInt>)base;

		impl = codeAsm.pushType(cllr::Instruction(p->isSigned ? cllr::Opcode::TYPE_INT_SIGN : cllr::Opcode::TYPE_INT_UNSIGN, { p->width }));

	}

	return impl;
}
