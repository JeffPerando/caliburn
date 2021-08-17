
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ValueStatement : public Statement
	{
		ValueStatement(StatementType t) : Statement(t) {}
		virtual ~ValueStatement() {}

		virtual bool isLValue() = 0;

	};

	struct NullValueStatement : public ValueStatement
	{
		ParsedType* resultType;

		NullValueStatement(ParsedType* type) :
			ValueStatement(StatementType::LITERAL), resultType(type)
		{}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			uint32_t type = codeAsm->resolveType(resultType)->typeDeclSpirV(codeAsm);
			uint32_t ssa = codeAsm->newAssign();
			codeAsm->pushAll({spirv::OpConstantNull(), type, ssa});
			return ssa;
		}

	};

	struct LiteralIntStatement : public ValueStatement
	{
		std::string lit = "";
		int32_t value = 0;

		LiteralIntStatement(std::string i) : ValueStatement(StatementType::LITERAL), lit(i){}

		bool isLValue()
		{
			return false;
		}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			
		}

	};

}
