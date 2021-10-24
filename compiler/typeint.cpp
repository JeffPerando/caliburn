
#include "allasm.h"
#include "typeint.h"

using namespace caliburn;

uint32_t TypeInt::getSizeBytes() const
{
	return intBits / 8 + ((intBits & 0b111) != 0);
}

uint32_t TypeInt::getAlignBytes() const
{
	return getSizeBytes();
}

void TypeInt::getConvertibleTypes(std::set<CompiledType*>* types, CaliburnAssembler* codeAsm)
{
	auto its = codeAsm->getAllIntTypes();
	auto fts = codeAsm->getAllFloatTypes();

	for (auto it : *its)
	{
		types->emplace(it);

	}

	for (auto ft : *fts)
	{
		types->emplace(ft);

	}

}

TypeCompat TypeInt::isCompatible(Operator op, CompiledType* rType) const
{
	if (rType == nullptr)
	{
		if ((op == Operator::NEGATE || op == Operator::ABS)
			&& !hasA(TypeAttrib::SIGNED))
		{
			return TypeCompat::INCOMPATIBLE_OP;
		}

		return TypeCompat::COMPATIBLE;
	}

	if (rType->category != TypeCategory::PRIMITIVE)
	{
		return TypeCompat::INCOMPATIBLE_TYPE;
	}
	if (rType->hasA(TypeAttrib::FLOAT) && this->getSizeBytes() > (MAX_FLOAT_BITS / 8))
	{
		return TypeCompat::INCOMPATIBLE_TYPE;
	}

	//This giant switch acts as a list of supported operators.
	switch (op)
	{
		case Operator::ADD:
		case Operator::SUB:
		case Operator::MUL:
		case Operator::DIV:
		case Operator::MOD:
		//case Operator::POW:
		case Operator::BIT_AND:
		case Operator::BIT_OR:
		case Operator::BIT_XOR:
		case Operator::SHIFT_LEFT:
		case Operator::SHIFT_RIGHT:
		case Operator::INTDIV:
		case Operator::ABS: return TypeCompat::COMPATIBLE;
	}

	return TypeCompat::INCOMPATIBLE_OP;
}

uint32_t TypeInt::typeDeclSpirV(SpirVAssembler* codeAsm)
{
	if (ssa != 0)
	{
		return ssa;
	}

	ssa = codeAsm->newAssign();
	codeAsm->pushAll({ spirv::OpTypeInt(), ssa, intBits, (uint32_t)hasA(TypeAttrib::SIGNED) });
	return ssa;
}

uint32_t TypeInt::mathOpSpirV(SpirVAssembler* codeAsm,
	uint32_t lhs, Operator op,
	CompiledType* rType, uint32_t rhs,
	CompiledType*& endType) const
{
	uint32_t resultTypeSSA = this->ssa;
	uint32_t result = codeAsm->newAssign();

	if (rType->hasA(TypeAttrib::FLOAT) || op == Operator::DIV)
	{
		CompiledType* fpLHS = codeAsm->getFloatType(this->getSizeBytes());
		uint32_t fpLHSTypeSSA = fpLHS->getSSA();

		uint32_t converted = codeAsm->newAssign();
		uint32_t convertOp = spirv::OpConvertSToF();

		if (!rType->hasA(TypeAttrib::SIGNED))
		{
			convertOp = spirv::OpConvertUToF();
		}

		codeAsm->pushAll({ convertOp, fpLHSTypeSSA, converted, lhs });

		return fpLHS->mathOpSpirV(codeAsm, converted, op, rType, rhs, endType);
	}

	if (this->getSizeBytes() != rType->getSizeBytes())
	{
		uint32_t converted = codeAsm->newAssign();
		uint32_t convertOp = spirv::OpSConvert();

		if (this->getSizeBytes() > rType->getSizeBytes())
		{
			//lhs is wider, convert rhs to lhs width

			if (!hasA(TypeAttrib::SIGNED))
			{
				convertOp = spirv::OpUConvert();
			}

			codeAsm->pushAll({ convertOp, resultTypeSSA, converted, rhs });

			rhs = converted;
			endType = (TypeInt*)this;

		}
		else
		{
			//rhs is wider, convert lhs to rhs width
			resultTypeSSA = rType->getSSA();

			if (!rType->hasA(TypeAttrib::SIGNED))
			{
				convertOp = spirv::OpUConvert();
			}

			codeAsm->pushAll({ convertOp, resultTypeSSA, converted, lhs });

			lhs = converted;
			endType = rType;

		}

	}
	else
	{
		endType = (TypeInt*)this;
	}

	SpvOp opcode = spirv::OpNop();

	switch (op)
	{
		case Operator::ADD: opcode = spirv::OpIAdd(); break;
		case Operator::SUB: opcode = spirv::OpISub(); break;
		case Operator::MUL: opcode = spirv::OpIMul(); break;
		case Operator::INTDIV:
			if (!rType->hasA(TypeAttrib::SIGNED) && !hasA(TypeAttrib::SIGNED))
				opcode = spirv::OpUDiv();
			else opcode = spirv::OpSDiv();
			break;
		case Operator::MOD:
			if (!rType->hasA(TypeAttrib::SIGNED) && !hasA(TypeAttrib::SIGNED))
				opcode = spirv::OpUMod();
			else opcode = spirv::OpSMod();
			break;
		case Operator::BIT_AND: opcode = spirv::OpBitwiseAnd(); break;
		case Operator::BIT_OR: opcode = spirv::OpBitwiseOr(); break;
		case Operator::BIT_XOR: opcode = spirv::OpBitwiseXor(); break;
		case Operator::SHIFT_LEFT: opcode = spirv::OpShiftLeftLogical(); break;
		case Operator::SHIFT_RIGHT: opcode = spirv::OpShiftRightArithmetic(); break;
		//case Operator::POW:
	}

	if (opcode == spirv::OpNop())
	{
		if (op == Operator::ARRAY_ACCESS)
		{
			uint32_t shifted = codeAsm->newAssign();
			codeAsm->pushAll({
				spirv::OpShiftRightLogical(), resultTypeSSA, shifted, lhs, rhs });
			codeAsm->pushAll({ spirv::OpBitwiseAnd(),
				resultTypeSSA, result, shifted,
				/*codeAsm->getOrPushIntConst(1, intBits, hasA(TypeAttrib::SIGNED)).value */});
			return result;
		}

		//TODO complain
	}

	codeAsm->pushAll({ opcode, resultTypeSSA, result, lhs, rhs });

	return result;
}

uint32_t TypeInt::mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& resultType) const
{
	return 0;
}