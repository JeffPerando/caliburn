
#pragma once

#include "value.h"

namespace caliburn
{
	struct LocalVariable : public Variable
	{
		LocalVariable(Statement* parent, Token* varName, ParsedType* hint, Value* init, bool isImmut) : Variable(parent, varName, hint, init, isImmut) {}
		LocalVariable(const LocalVariable& rhs) : Variable(rhs) {}

		bool isLValue() const override
		{
			return true;
		}

		Token* firstTkn() const override
		{
			return name;
		}

		Token* lastTkn() const override
		{
			return name;
		}

		void resolveSymbols(const SymbolTable& table) override
		{

		}

		cllr::SSA emitLoadCLLR(cllr::Assembler& codeAsm) override
		{

		}

		void emitStoreCLLR(cllr::Assembler& codeAsm, cllr::SSA value) override
		{

		}

	};

	struct GlobalVariable : public Variable
	{

	};

	struct FunctionArgument : public Variable
	{


	};

	struct ShaderIOVariable : public Variable
	{
		
	};

}
