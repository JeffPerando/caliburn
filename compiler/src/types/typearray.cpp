
#include "types/typearray.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

sptr<cllr::LowType> TypeArray::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (!sig->canApply(*gArgs))
	{
		//TODO complain
	}

	auto t = gArgs->getType(0);
	auto elemType = t->resolve(table, codeAsm);
	auto len = gArgs->getConst(1);
	auto lenRes = len->emitValueCLLR(table, codeAsm);
	cllr::TypedSSA length;

	MATCH(lenRes, cllr::TypedSSA, lenPtr)
	{
		length = *lenPtr;
	}
	else
	{
		codeAsm.errors->err("Invalid array length", *len);
	}

	//FIXME typecheck the length too .-.
	//FIXME I need to be able to extract an int constant from a Value
	//Technically I can just check the SSA ID for a constant, but that feels a lil hacky
	//Lowkey need to implement an interpreter anyway too...
	return codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_ARRAY, {}, { elemType->id, length.value }));
}
