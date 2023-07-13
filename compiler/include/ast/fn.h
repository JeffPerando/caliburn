
#pragma once

#include <map>

#include "basic.h"
#include "langcore.h"

#include "ast.h"
#include "generics.h"
#include "symbols.h"
#include "var.h"

#include "cllr/cllrasm.h"

namespace caliburn
{
	class Parser;
	struct Function;
	
	struct FunctionSignature
	{
		std::vector<sptr<FnArgVariable>> args;
		sptr<GenericSignature> genSig;
		sptr<ParsedType> returnType;

	};

	struct FunctionImpl : cllr::Emitter
	{
		friend class Parser;
		friend struct Function;

	private:
		const ptr<Function> parent;
		const sptr<GenericArguments> genArgs;

		sptr<SymbolTable> table = nullptr;

	public:
		cllr::SSA id = 0;

		FunctionImpl(ptr<Generic<FunctionImpl>> parent, sptr<GenericArguments> gArgs) : parent((ptr<Function>)parent), genArgs(gArgs) {}
		virtual ~FunctionImpl() {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		virtual cllr::TypedSSA call(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, ref<const std::vector<sptr<Value>>> args);

	};

	struct Function : Generic<FunctionImpl>
	{
		friend class Parser;
		friend struct FunctionImpl;

		const sptr<FunctionSignature> sig;

		uptr<ScopeStatement> code = nullptr;

		Function(sptr<FunctionSignature> sig) : Generic(sig->genSig), sig(sig) {}
		virtual ~Function() {}

	};

	struct FunctionName
	{
		std::map<sptr<FunctionSignature>, sptr<Function>> fns;

		FunctionName() = default;
		virtual ~FunctionName() {}

	};

}
