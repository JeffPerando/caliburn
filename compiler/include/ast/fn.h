
#pragma once

#include <map>

#include "basic.h"
#include "langcore.h"

#include "ast/generics.h"
#include "ast/symbols.h"
#include "ast/var.h"

#include "cllr/cllrasm.h"

namespace caliburn
{
	class Parser;
	struct ScopeStatement;

	struct FunctionSignature
	{
		std::vector<sptr<Variable>> args;
		sptr<GenericSignature> genSig;
		sptr<ParsedType> returnType;

	};

	class FunctionImpl : public cllr::Emitter
	{
		friend class Parser;
		friend class Function;

		const ptr<Function> parent;

		sptr<SymbolTable> table = nullptr;

		const sptr<GenericArguments> genArgs;

	public:
		cllr::SSA id = 0;

		FunctionImpl(ptr<Function> parent, sptr<GenericArguments> gArgs) : parent(parent), genArgs(gArgs) {}
		virtual ~FunctionImpl() {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		virtual cllr::SSA call(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, ref<const std::vector<sptr<Value>>> args);

	};

	class Function : public Generic<FunctionImpl>
	{
		friend class Parser;
		friend class FunctionImpl;

		const sptr<FunctionSignature> sig;

		uptr<ScopeStatement> code = nullptr;

	public:
		Function(sptr<FunctionSignature> sig) : Generic(sig->genSig), sig(sig) {}
		virtual ~Function() {}

	};

}
