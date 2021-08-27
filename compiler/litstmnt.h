
#pragma once

#include "valuestmnt.h"

namespace caliburn
{
	struct NullValueStatement : public ValueStatement
	{
		ParsedType* resultType;

		NullValueStatement(ParsedType* type) :
			ValueStatement(StatementType::LITERAL), resultType(type)
		{}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm)
		{
			uint32_t type = codeAsm->resolveType(resultType)->typeDeclSpirV(codeAsm);
			uint32_t ssa = codeAsm->newAssign();
			codeAsm->pushAll({ spirv::OpConstantNull(), type, ssa });
			return ssa;
		}

	};

	struct LiteralStatement : public ValueStatement
	{
		std::string lit;

		LiteralStatement(std::string val) : ValueStatement(StatementType::LITERAL), lit(val) {}

		bool isLValue()
		{
			return false;
		}

	};

	struct LiteralIntStatement : public LiteralStatement
	{
		int32_t value = 0;

		LiteralIntStatement(std::string i) : LiteralStatement(i) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm)
		{
			value = std::stoi(lit);
			uint32_t ssa = codeAsm->newAssign();

		}

	};

}