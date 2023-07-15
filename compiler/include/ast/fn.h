
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
	struct Function;
	
	enum class FnType : uint32_t
	{
		FUNCTION,
		CONSTRUCTOR,
		DESTRUCTOR,
		CONVERTER,
		OP_OVERLOAD,
		MEMBER_FN,
	};

	struct FunctionSignature
	{
		std::vector<sptr<FnArgVariable>> args;
		sptr<GenericSignature> genSig;
		sptr<ParsedType> returnType;

	};

	struct FunctionImpl : cllr::Emitter
	{
		const ptr<Function> parent;
		const sptr<GenericArguments> genArgs;

		FunctionImpl(ptr<Generic<FunctionImpl>> parent, sptr<GenericArguments> gArgs) : parent((ptr<Function>)parent), genArgs(gArgs) {}
		virtual ~FunctionImpl() {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		virtual cllr::TypedSSA call(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, ref<const std::vector<sptr<Value>>> args);

	private:
		sptr<SymbolTable> fnImplTable = nullptr;
		cllr::SSA id = 0;

	};

	struct Function : Generic<FunctionImpl>
	{
		const sptr<FunctionSignature> sig;

		sptr<Token> name = nullptr;
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

	struct Method : Function
	{
		Method(sptr<FunctionSignature> sig) : Function(sig)
		{

		}

	};

}
