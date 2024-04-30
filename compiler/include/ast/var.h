
#pragma once

#include "types/type.h"

namespace caliburn
{
	struct FnArg
	{
		sptr<ParsedType> typeHint;
		std::string name;

	};

	struct LocalVariable : Variable
	{
		LocalVariable(in<std::string> name) : Variable(name) {}
		LocalVariable(in<ParsedVar> v) : Variable(v) {}
		virtual ~LocalVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target, cllr::TypedSSA value) override;

	};

	struct MemberVariable : Variable
	{
		sptr<BaseType> parent = nullptr;
		uint32_t memberIndex = 0;

		MemberVariable(in<std::string> name) : Variable(name) {}
		virtual ~MemberVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target, cllr::TypedSSA value) override;

	};

	struct GlobalVariable : Variable
	{
		GlobalVariable(in<std::string> name) : Variable(name)
		{
			isConst = true;
		}
		virtual ~GlobalVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target, cllr::TypedSSA value) override;

	};

	struct FnArgVariable : Variable
	{
		const uint32_t argIndex;

		FnArgVariable(in<FnArg> data, uint32_t i) : Variable(data.name), argIndex(i)
		{
			typeHint = data.typeHint;

		}
		virtual ~FnArgVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target, cllr::TypedSSA value) override;

	};

	enum class ShaderIOVarType
	{
		UNKNOWN,
		INPUT,
		OUTPUT
	};

	struct ShaderIOVariable : Variable
	{
		ShaderIOVarType ioType = ShaderIOVarType::UNKNOWN;
		uint32_t ioIndex = 0;

		ShaderIOVariable(std::string n) : Variable(n) {}
		ShaderIOVariable(in<ParsedVar> v) : Variable(v) {}
		ShaderIOVariable(ShaderIOVarType t, in<FnArg> v) : Variable(v.name), ioType(t)
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

		cllr::TypedSSA emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target, cllr::TypedSSA value) override;

	};

	struct DescriptorVariable : Variable
	{
		DescriptorVariable(std::string n) : Variable(n) {}
		DescriptorVariable(sptr<Token> n) : Variable(n->str)
		{
			nameTkn = n;
		}
		virtual ~DescriptorVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA target, cllr::TypedSSA value) override;

	};

}
