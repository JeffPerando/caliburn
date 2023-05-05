
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

		cllr::SSA emitLoadCLLR(cllr::Assembler& codeAsm, cllr::SSA target) override
		{
			return 0;
		}

		void emitStoreCLLR(cllr::Assembler& codeAsm, cllr::SSA target, cllr::SSA value) override
		{

		}

	};

	struct MemberVariable : public Variable
	{
		ptr<Type> parent = nullptr;
		uint32_t memberIndex = 0;

		MemberVariable(Token* varName, ParsedType* hint, Value* init, bool isImmut) :
			Variable(varName, hint, init, isImmut) {}
		MemberVariable(const MemberVariable& rhs) :
			Variable(rhs),
			parent(rhs.parent),
			memberIndex(rhs.memberIndex) {}
		virtual ~MemberVariable() {}

		Token* firstTkn() const override
		{
			return name;
		}

		Token* lastTkn() const override
		{
			return name;
		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			codeAsm.push(0, cllr::Opcode::STRUCT_MEMBER, { memberIndex }, { parent->id, type->id });

		}

		cllr::SSA emitLoadCLLR(cllr::Assembler& codeAsm, cllr::SSA target) override
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_MEMBER, { memberIndex }, { target, type->id });
		}

		void emitStoreCLLR(cllr::Assembler& codeAsm, cllr::SSA target, cllr::SSA value) override
		{
			auto memberSSA = emitLoadCLLR(codeAsm, target);

			codeAsm.push(0, cllr::Opcode::ASSIGN, {}, { memberSSA, value });

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
