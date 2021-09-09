
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ClassStatement : public Statement
	{
		const Token* name;
		ParsedType* ext = nullptr;
		std::vector<ParsedType*> interfaces;
		std::vector<Statement*> members;
		
		ClassStatement(Token* tName) : name(tName), Statement(StatementType::CLASS) {}

	};

	struct StructStatement : public Statement
	{
		const Token* name;
		ParsedType* ext = nullptr;
		std::vector<Statement*> members;

		StructStatement(Token* tName) : name(tName), Statement(StatementType::STRUCT) {}

	};

	struct ShaderStatement : public Statement
	{
		const Token* name;
		ParsedType* ext = nullptr;
		std::vector<ParsedType*> descriptors;
		std::vector<Statement*> members;

		ShaderStatement(Token* tName) : name(tName), Statement(StatementType::SHADER) {}

	};

	struct DescriptorStatement : public Statement
	{
		const Token* name;
		ParsedType* ext = nullptr;
		std::vector<Statement*> members;

		DescriptorStatement(Token* tName) : name(tName), Statement(StatementType::DESCRIPTOR) {}

	};

}
