
#pragma once

#include "ast/type.h"

namespace caliburn
{
	struct FnArg
	{
		sptr<ParsedType> typeHint;
		std::string_view name;

	};

	struct LocalVariable : Variable
	{
		LocalVariable(std::string_view name) : Variable(name) {}
		LocalVariable(in<ParsedVar> v) : Variable(v) {}
		virtual ~LocalVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			return cllr::TypedSSA();
		}

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA value) override {}

	};

	struct GlobalVariable : Variable
	{
		GlobalVariable(std::string_view name) : Variable(name)
		{
			isConst = true;
		}
		virtual ~GlobalVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override
		{
			return cllr::TypedSSA();
		}

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA value) override {}

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

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA value) override;

	};

	struct ShaderIOVariable : Variable
	{
		ShaderIOVariable(std::string_view n) : Variable(n) {}
		ShaderIOVariable(in<ParsedVar> v) : Variable(v) {}
		ShaderIOVariable(in<FnArg> v) : Variable(v.name)
		{
			typeHint = v.typeHint;

		}
		
		virtual ~ShaderIOVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA value) override;

	};

	struct DescriptorVariable : Variable
	{
		DescriptorVariable(std::string_view n) : Variable(n) {}
		DescriptorVariable(in<Token> n) : Variable(n.str)
		{
			nameTkn = n;
		}
		virtual ~DescriptorVariable() {}

		void prettyPrint(out<std::stringstream> ss) const override;

		cllr::TypedSSA emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		void emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA value) override;

	};

}
