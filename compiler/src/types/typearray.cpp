
#include "types/typearray.h"

using namespace caliburn;

cllr::SSA RealArray::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	/*
	auto innerID = ((TypeArray*)base)->innerType->emitDeclCLLR(table, codeAsm);

	//TODO length????
	return codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_ARRAY, {}, { innerID }));
	*/
	return 0;
}
