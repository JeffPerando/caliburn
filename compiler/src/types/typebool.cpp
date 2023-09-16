
#include "types/typebool.h"

using namespace caliburn;

cllr::SSA RealBool::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id == 0)
	{
		id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::TYPE_BOOL));
	}

	return id;
}

/*
void TypeBool::getConvertibleTypes(std::set<ConcreteType*>* types, CaliburnAssembler* codeAsm)
{
	auto its = codeAsm->getAllIntTypes();

	for (auto it : *its)
	{
		types->emplace(it);
	}

}

TypeCompat TypeBool::isCompatible(Operator op, sptr<BaseType> rType) const
{
	if (!rType)
	{
		if (op == Operator::BOOL_NOT)
		{
			return TypeCompat::COMPATIBLE;
		}

		return TypeCompat::INCOMPATIBLE_OP;
	}
	
	if (op == Operator::ADD || op == Operator::MUL)
	{
		if (rType->category == TypeCategory::INT)
		{
			return TypeCompat::COMPATIBLE;
		}

		return TypeCompat::INCOMPATIBLE_TYPE;
	}

	if (rType->category != TypeCategory::BOOLEAN)
	{
		return TypeCompat::INCOMPATIBLE_TYPE;
	}

	switch (op)
	{
	case Operator::COMP_EQ:
	case Operator::COMP_NEQ:
	case Operator::AND:
	case Operator::OR: return TypeCompat::COMPATIBLE;
	}

	return TypeCompat::INCOMPATIBLE_OP;
}

uint32_t TypeBool::mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, ConcreteType* rType, uint32_t rvalueSSA, ConcreteType*& endType) const
{
	uint32_t lhs = lvalueSSA;

	if (op == Operator::ADD || op == Operator::MUL)
	{

	}

	return 0;
}

uint32_t TypeBool::mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, ConcreteType*& endType) const
{
	return 0;
}
*/