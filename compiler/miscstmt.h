
#pragma once

#include "statement.h"

namespace caliburn
{
	struct ImportStatement : public Statement
	{
		const Token* importedModule;
		const Token* importAlias;

		ImportStatement(Token* m, Token* a) : importedModule(m), importAlias(a),
			Statement(StatementType::IMPORT) {}
		~ImportStatement() {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			//No complaining
			return 0;
		}

	};

	struct TypedefStatement : public Statement
	{
		std::string name = "";
		ParsedType* actualType = nullptr;

		TypedefStatement() : Statement(StatementType::TYPEDEF) {}

		uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms)
		{
			return 0;
		}

	};

}
