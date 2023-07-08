
#include "types/typevector.h"

using namespace caliburn;

cllr::SSA RealVector::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	auto& t = genArgs->args[0];

	if (auto& ptype = std::get<sptr<ParsedType>>(t))
	{
		if (auto type = ptype->resolve(table))
		{
			return codeAsm.pushNew(cllr::Opcode::TYPE_VECTOR, { ((ptr<TypeVector>)base)->elements }, { type->emitDeclCLLR(table, codeAsm) });
		}

	}

	//TODO complain
	return 0;
}

/*
TypeCompat TypeVector::isCompatible(Operator op, sptr<BaseType> rType) const
{
	if (rType == nullptr)
	{
		return inner->isCompatible(op, nullptr);
	}

	if (rType->category == TypeCategory::VECTOR || rType->category == inner->category)
	{
		switch (op)
		{
			case Operator::BIT_AND:
			case Operator::BIT_NEG:
			case Operator::BIT_OR:
			case Operator::BIT_XOR: return TypeCompat::INCOMPATIBLE_OP;
		}

		return TypeCompat::COMPATIBLE;
	}

	return TypeCompat::INCOMPATIBLE_TYPE;
}
*/
