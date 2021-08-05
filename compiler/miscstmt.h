
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ImportStatement : public Statement
	{
		std::string const importedModule;

		ImportStatement(std::string m) : importedModule(m),
			Statement(StatementType::IMPORT) {}
		~ImportStatement() {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			//No complaining
			return 0;
		}

	};

	struct UsingStatement : public Statement
	{
		std::string const usedNamespace;

		UsingStatement(std::string n) : usedNamespace(n),
			Statement(StatementType::USING) {}
		~UsingStatement() {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			return 0;
		}

	};

	struct TypedefStatement : public Statement
	{
		std::string name = "";
		ParsedType* actualType = nullptr;

		TypedefStatement() : Statement(StatementType::TYPEDEF) {}

		uint32_t toSPIRV(SpirVAssembler* codeAsm)
		{
			return 0;
		}

	};

}
