
#include "types/typevoid.h"

using namespace caliburn;

cllr::SSA RealVoid::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	return codeAsm.pushNew(cllr::Opcode::TYPE_VOID, {}, {});
}
