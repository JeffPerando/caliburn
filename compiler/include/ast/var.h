
#pragma once

#include "types/type.h"

namespace caliburn
{
	struct ParsedFnArg
	{
		sptr<ParsedType> typeHint;
		sptr<Token> name;
	};

	struct LocalVariable : Variable
	{
		LocalVariable() : Variable() {}
		LocalVariable(in<ParsedVar> v) : Variable(v) {}
		virtual ~LocalVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

	struct MemberVariable : Variable
	{
		sptr<RealType> parent = nullptr;
		uint32_t memberIndex = 0;

		MemberVariable() : Variable() {}
		virtual ~MemberVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

	struct GlobalVariable : Variable
	{
		GlobalVariable() : Variable()
		{
			isConst = true;
		}
		virtual ~GlobalVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

	struct FnArgVariable : Variable
	{
		const uint32_t argIndex;

		FnArgVariable(in<ParsedFnArg> pArgs, uint32_t i) : Variable(), argIndex(i)
		{
			typeHint = pArgs.typeHint;
			nameTkn = pArgs.name;

		}
		virtual ~FnArgVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

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

		ShaderIOVariable(std::string n) : Variable(), name(n) {}
		ShaderIOVariable(in<ParsedVar> v) : Variable(v), name(v.name->str) {}
		ShaderIOVariable(ShaderIOVarType t, out<ParsedFnArg> v) : Variable(), ioType(t), name(v.name->str)
		{
			typeHint = v.typeHint;

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

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

	struct DescriptorVariable : Variable
	{
		const std::string name;

		DescriptorVariable(std::string n) : name(n) {}
		DescriptorVariable(sptr<Token> n) : name(n->str)
		{
			nameTkn = n;
		}
		virtual ~DescriptorVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target) override;

		void emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value) override;

	};

}
