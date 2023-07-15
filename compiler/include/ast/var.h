
#pragma once

#include "types/type.h"

namespace caliburn
{
	struct LocalVariable : Variable
	{
		LocalVariable() : Variable() {}
		virtual ~LocalVariable() {}

		void prettyPrint(ref<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

	struct MemberVariable : Variable
	{
		sptr<RealType> parent = nullptr;
		uint32_t memberIndex = 0;

		MemberVariable() : Variable() {}
		virtual ~MemberVariable() {}

		void prettyPrint(ref<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

	struct GlobalVariable : Variable
	{
		GlobalVariable() : Variable()
		{
			isConst = true;
		}
		virtual ~GlobalVariable() {}

		void prettyPrint(ref<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

	struct FnArgVariable : Variable
	{
		const uint32_t argIndex;

		FnArgVariable(uint32_t i) : Variable(), argIndex(i) {}
		virtual ~FnArgVariable() {}

		void prettyPrint(ref<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

	enum class ShaderIOVarType
	{
		UNKNOWN,
		INPUT,
		OUTPUT
	};

	struct ShaderIOVariable : Variable
	{
	private:
		ShaderIOVarType ioType = ShaderIOVarType::UNKNOWN;
		uint32_t ioIndex = 0;

	public:
		const std::string name;

		ShaderIOVariable(std::string n) : name(n) {}
		ShaderIOVariable(sptr<Token> n) : name(n->str)
		{
			nameTkn = n;
		}
		virtual ~ShaderIOVariable() {}

		ShaderIOVarType getIOType()
		{
			return ioType;
		}

		uint32_t getIndex()
		{
			return ioIndex;
		}

		void prettyPrint(ref<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

}
