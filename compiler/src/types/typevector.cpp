
#include "types/typevector.h"

using namespace caliburn;

cllr::SSA RealVector::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id == 0)
	{
		auto& t = genArgs->args[0];

		if (auto& ptype = std::get<sptr<ParsedType>>(t))
		{
			if (auto type = ptype->resolve(table))
			{
				/* FIXME
				((ptr<TypeVector>)base)->elements
				returns garbage
				*/

				id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::TYPE_VECTOR, { length }, { type->emitDeclCLLR(table, codeAsm) }));
				return id;
			}

		}

		//TODO complain
	}

	return id;
}

Member TypeVector::getMember(in<std::string> name) const
{
	//TODO swizzle
	return Member();
}
