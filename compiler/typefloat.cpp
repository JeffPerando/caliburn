
#include "assembler.h"
#include "typefloat.h"

using namespace caliburn;

uint32_t FloatType::getSizeBytes() const
{
	return floatBits / 8 + (floatBits & 0b111 != 0);
}

uint32_t FloatType::getAlignBytes() const
{
	return getSizeBytes();
}

CompiledType* FloatType::clone()
{
	return new FloatType(floatBits);
}

TypeCompat FloatType::isCompatible(Operator op, CompiledType* rType) const
{
	if (rType == nullptr)
	{
		//can't do bitwise ops on a float, need to use bits() to convert
		if (op == Operator::BIT_NOT)
		{
			return TypeCompat::INCOMPATIBLE_OP;
		}

		return TypeCompat::COMPATIBLE;
	}

	if (rType->category == TypeCategory::PRIMITIVE)
	{
		switch (op)
		{
		case Operator::BIT_AND:
		case Operator::BIT_OR:
		case Operator::BIT_XOR:
		case Operator::APPEND:
		case Operator::ARRAY_ACCESS: return TypeCompat::INCOMPATIBLE_OP;
		default: return TypeCompat::COMPATIBLE;
		}

	}

	return TypeCompat::INCOMPATIBLE_TYPE;
}

uint32_t FloatType::typeDeclSpirV(SpirVAssembler* codeAsm)
{
	if (ssa != 0)
	{
		return ssa;
	}

	ssa = codeAsm->newAssign();
	codeAsm->pushAll({ spirv::OpTypeFloat(), ssa, floatBits });
	return ssa;
}

uint32_t FloatType::mathOpSpirV(SpirVAssembler* codeAsm, uint32_t lvalueSSA, Operator op, CompiledType* rType, uint32_t rvalueSSA, CompiledType*& endType) const
{
	uint32_t lhs = lvalueSSA;
	uint32_t rhs = rvalueSSA;
	uint32_t resultTypeSSA = this->ssa;
	uint32_t result = codeAsm->newAssign();

	if (!rType->hasA(TypeAttrib::FLOAT))
	{
		FloatType* fpRHS = codeAsm->getFloatType(rType->getSizeBytes());
		uint32_t fpRHSTypeSSA = fpRHS->getSSA();

		uint32_t converted = codeAsm->newAssign();
		uint32_t convertOp = spirv::OpConvertSToF();

		if (!rType->hasA(TypeAttrib::SIGNED))
		{
			convertOp = spirv::OpConvertUToF();
		}

		codeAsm->pushAll({ convertOp, fpRHSTypeSSA, converted, rhs });
		rhs = converted;

	}

	if (this->getSizeBytes() != rType->getSizeBytes())
	{
		uint32_t converted = codeAsm->newAssign();

		if (this->getSizeBytes() > rType->getSizeBytes())
		{
			//lhs is wider, convert rhs to lhs width

			codeAsm->pushAll({ spirv::OpFConvert(), resultTypeSSA, converted, rhs });

			rhs = converted;
			endType = (FloatType*)this;

		}
		else
		{
			//rhs is wider, convert lhs to rhs width
			resultTypeSSA = rType->getSSA();

			codeAsm->pushAll({ spirv::OpFConvert(), resultTypeSSA, converted, lhs });

			lhs = converted;
			endType = rType;

		}

	}
	else
	{
		endType = (FloatType*)this;
	}

	SpvOp opcode = spirv::OpNop();

	switch (op)
	{
		case Operator::ADD: opcode = spirv::OpFAdd(); break;
		case Operator::SUB: opcode = spirv::OpFSub(); break;
		case Operator::MUL: opcode = spirv::OpFMul(); break;
		case Operator::INTDIV:
		case Operator::DIV: opcode = spirv::OpFDiv(); break;
		case Operator::MOD: opcode = spirv::OpFMod(); break;
		//case Operator::POW:
		default: return 0;//TODO complain
	}

	codeAsm->pushAll({ opcode, resultTypeSSA, result, lhs, rhs });

	if (op == Operator::INTDIV)
	{
		auto intResultType = codeAsm->getIntType(this->getSizeBytes(), true);
		uint32_t intDivResult = codeAsm->newAssign();
		codeAsm->pushAll({ spirv::OpConvertFToS(), intResultType->getSSA(), intDivResult, result });
		endType = intResultType;
		return intDivResult;
	}

	return result;
}

uint32_t FloatType::mathOpSoloSpirV(SpirVAssembler* codeAsm, Operator op, uint32_t ssa, CompiledType*& endType) const
{
	return 0;
}
