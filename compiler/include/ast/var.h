
#pragma once

#include "values.h"

namespace caliburn
{
	struct LocalVariable : public Variable
	{
		LocalVariable(sptr<Token> varName, uptr<ParsedType> hint, uptr<Value> init, bool isImmut) : Variable(varName, std::move(hint), std::move(init), isImmut) {}
		virtual ~LocalVariable() {}

		sptr<Token> firstTkn() const override
		{
			return name;
		}

		sptr<Token> lastTkn() const override
		{
			return name;
		}

		void resolveSymbols(sptr<const SymbolTable> mod) override
		{

		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{

		}

		cllr::SSA emitLoadCLLR(ref<cllr::Assembler> codeAsm, cllr::SSA target) override
		{
			return 0;
		}

		void emitStoreCLLR(ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override
		{

		}

	};

	struct MemberVariable : public Variable
	{
		sptr<Type> parent = nullptr;
		uint32_t memberIndex = 0;

		MemberVariable(sptr<Token> varName, uptr<ParsedType> hint, uptr<Value> init, bool isImmut) :
			Variable(varName, std::move(hint), std::move(init), isImmut) {}
		virtual ~MemberVariable() {}

		sptr<Token> firstTkn() const override
		{
			return name;
		}

		sptr<Token> lastTkn() const override
		{
			return name;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			typeHint->prettyPrint(ss);

			ss << ' ';

			ss << name->str;

			ss << ' ';

			if (initValue != nullptr)
			{
				ss << "= ";

				initValue->prettyPrint(ss);

			}

			ss << ';';

		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			codeAsm.push(0, cllr::Opcode::STRUCT_MEMBER, { memberIndex }, { parent->id, type->id });

		}

		cllr::SSA emitLoadCLLR(ref<cllr::Assembler> codeAsm, cllr::SSA target) override
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_MEMBER, { memberIndex }, { target, type->id });
		}

		void emitStoreCLLR(ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override
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
