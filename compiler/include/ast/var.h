
#pragma once

#include "types/type.h"

namespace caliburn
{
	class LocalVariable;
	class MemberVariable;
	class GlobalVariable;

	class LocalVariable : public Variable
	{
	public:
		LocalVariable(StmtModifiers mods, sptr<Token> start, sptr<Token> name, sptr<ParsedType> type, sptr<Value> initValue) :
			Variable(mods, start, name, type, initValue) {}
		virtual ~LocalVariable() {}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << (isConst ? "const" : "var");

			if (typeHint != nullptr)
			{
				ss << ": ";
				typeHint->prettyPrint(ss);

			}

			ss << ' ' << name->str;

			if (initValue != nullptr)
			{
				ss << " = ";
				initValue->prettyPrint(ss);
			}

		}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			cllr::SSA valID = 0, typeID = 0;

			if (initValue == nullptr)
			{
				//initValue = new_sptr<ZeroValue>();
			}

			valID = initValue->emitValueCLLR(table, codeAsm);
			
			if (typeHint != nullptr)
			{
				typeID = typeHint->resolve(table)->emitDeclCLLR(table, codeAsm);
				
				//TODO check for compatibility with initial value

			}

			id = codeAsm.pushNew(cllr::Opcode::VAR_LOCAL, { (uint32_t)mods }, { typeID, valID });

			return id;
		}

		cllr::SSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) override
		{
			return 0;
		}

		void emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override
		{

		}

	};

	class MemberVariable : public Variable
	{
	public:
		sptr<RealType> parent = nullptr;
		uint32_t memberIndex = 0;

		MemberVariable(StmtModifiers mods, sptr<Token> start, sptr<Token> name, sptr<ParsedType> typeHint, sptr<Value> initValue) :
			Variable(mods, start, name, typeHint, initValue) {}
		virtual ~MemberVariable() {}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			typeHint->prettyPrint(ss);

			ss << ' ';
			ss << name->str;

			if (initValue != nullptr)
			{
				ss << " = ";
				initValue->prettyPrint(ss);

			}

			ss << ';';

		}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			cllr::SSA valID = 0;

			if (initValue != nullptr)
			{
				valID = initValue->emitValueCLLR(table, codeAsm);
			}

			auto parentID = parent->emitDeclCLLR(table, codeAsm);
			auto typeID = typeHint->resolve(table)->emitDeclCLLR(table, codeAsm);

			codeAsm.push(0, cllr::Opcode::STRUCT_MEMBER, { memberIndex }, { parentID, typeID, valID });

			return 0;
		}

		cllr::SSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) override
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_MEMBER, { memberIndex }, { target });
		}

		void emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override
		{
			auto memberSSA = emitLoadCLLR(table, codeAsm, target);

			codeAsm.push(0, cllr::Opcode::ASSIGN, {}, { memberSSA, value });

		}

	};

	class GlobalVariable : public Variable
	{

	};

	class FunctionArgument : public Variable
	{


	};

	class ShaderIOVariable : public Variable
	{
		
	};

}
