
#include "assembler.h"
#include "typeint.h"

using namespace caliburn;

TypeCompat IntType::isCompatible(Operator op, CompiledType* rType) const
{
	if (rType == nullptr)
	{
		if ((op == Operator::NEGATE || op == Operator::ABS)
			&& !hasA(TypeAttrib::TA_SIGNED))
		{
			return TypeCompat::INCOMPATIBLE_OP;
		}

		return TypeCompat::COMPATIBLE;
	}

	if (rType->category == TypeCategory::PRIMITIVE)
	{
		if (rType->hasA(TypeAttrib::TA_FLOAT) && this->size > rType->size)
		{
			return TypeCompat::INCOMPATIBLE_TYPE;
		}

		//TODO POW is unsupported atm
		if (op == Operator::APPEND || op == Operator::POW)
		{
			return TypeCompat::INCOMPATIBLE_OP;
		}

		return TypeCompat::COMPATIBLE;
	}

	return TypeCompat::INCOMPATIBLE_TYPE;
}

uint32_t IntType::typeDeclSpirV(SpirVAssembler* codeAsm)
{
	if (ssa != 0)
	{
		return ssa;
	}

	ssa = codeAsm->newAssign();
	codeAsm->pushAll({ spirv::OpTypeInt(), ssa, size, (uint32_t)hasA(TypeAttrib::TA_SIGNED) });
	return ssa;
}

uint32_t IntType::mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const
{
	uint32_t lhs = lvalueSSA;
	uint32_t rhs = rvalueSSA;
	uint32_t resultTypeSSA = this->ssa;
	uint32_t result = codeAsm->newAssign();

	if (rType->hasA(TA_FLOAT) || op == Operator::DIV)
	{
		CompiledType* fpLHS = codeAsm->getFloatType(this->size);
		uint32_t fpLHSTypeSSA = fpLHS->getSSA();

		uint32_t converted = codeAsm->newAssign();
		uint32_t convertOp = spirv::OpConvertSToF();

		if (!rType->hasA(TypeAttrib::TA_SIGNED))
		{
			convertOp = spirv::OpConvertUToF();
		}

		codeAsm->pushAll({ convertOp, fpLHSTypeSSA, converted, lhs });

		return fpLHS->mathOpSpirV(codeAsm, converted, op, rType, rhs, endType);
	}

	if (this->size != rType->size)
	{
		uint32_t converted = codeAsm->newAssign();
		uint32_t convertOp = spirv::OpSConvert();

		if (this->size > rType->size)
		{
			//lhs is wider, convert rhs to lhs width

			if (!hasA(TypeAttrib::TA_SIGNED))
			{
				convertOp = spirv::OpUConvert();
			}

			codeAsm->pushAll({ convertOp, resultTypeSSA, converted, rhs });

			rhs = converted;
			endType = (IntType*)this;

		}
		else
		{
			//rhs is wider, convert lhs to rhs width
			resultTypeSSA = rType->getSSA();

			if (!rType->hasA(TypeAttrib::TA_SIGNED))
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
		endType = (IntType*)this;
	}

	SpvOp opcode = spirv::OpNop();

	switch (op)
	{
	case Operator::ADD: opcode = spirv::OpIAdd(); break;
	case Operator::SUB: opcode = spirv::OpISub(); break;
	case Operator::MUL: opcode = spirv::OpIMul(); break;
	case Operator::INTDIV:
		if (!rType->hasA(TA_SIGNED) && !hasA(TA_SIGNED))
			opcode = spirv::OpUDiv();
		else opcode = spirv::OpSDiv();
		break;
	case Operator::MOD:
		if (!rType->hasA(TA_SIGNED) && !hasA(TA_SIGNED))
			opcode = spirv::OpUMod();
		else opcode = spirv::OpSMod();
		break;
	case Operator::BIT_AND: opcode = spirv::OpBitwiseAnd(); break;
	case Operator::BIT_OR: opcode = spirv::OpBitwiseOr(); break;
	case Operator::BIT_XOR: opcode = spirv::OpBitwiseXor(); break;
		//case Operator::POW:
	}

	if (opcode == spirv::OpNop())
	{
		if (op == Operator::ARRAY_ACCESS)
		{
			uint32_t shifted = codeAsm->newAssign();
			codeAsm->pushAll({
				spirv::OpShiftRightLogical(), resultTypeSSA, shifted, lvalueSSA, rvalueSSA
				});
			codeAsm->pushAll({ spirv::OpBitwiseAnd(),
				resultTypeSSA,
				result,
				shifted, codeAsm->getOrPushIntConst(1, this->size, this->hasA(TypeAttrib::TA_SIGNED)).value });
			return result;
		}

		//TODO complain
	}

	codeAsm->pushAll({ opcode, resultTypeSSA, result, lhs, rhs });

	return result;
}

uint32_t IntType::mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& resultType) const
{
	return 0;
}