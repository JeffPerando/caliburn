
#pragma once

#include "values.h"

namespace caliburn
{
	struct LocalVariable : public Variable
	{
		LocalVariable(Token* varName, ParsedType* hint, Value* init, bool isImmut) : Variable(varName, hint, init, isImmut) {}
		LocalVariable(const LocalVariable& rhs) : Variable(rhs) {}
		virtual ~LocalVariable() {}

		Token* firstTkn() const override
		{
			return name;
		}

		Token* lastTkn() const override
		{
			return name;
		}

		void resolveSymbols(ref<const SymbolTable> mod) override
		{

		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
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
