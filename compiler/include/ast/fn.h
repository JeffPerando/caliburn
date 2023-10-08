
#pragma once

#include <map>

#include "basic.h"
#include "langcore.h"

#include "ast.h"
#include "generics.h"
#include "symbols.h"
#include "var.h"

#include "cllr/cllrasm.h"

#include "types/type.h"

namespace caliburn
{
	struct Function;
	
	enum class FnType : uint32_t
	{
		FUNCTION,
		CONSTRUCTOR,
		DESTRUCTOR,
		OP_OVERLOAD,
		CONVERTER
	};

	static const std::map<std::string, FnType> FN_TYPES = {
		{"def", FnType::FUNCTION},
		{"construct", FnType::CONSTRUCTOR},
		{"destruct", FnType::DESTRUCTOR},
		{"op", FnType::OP_OVERLOAD}
	};

	struct ParsedFn
	{
		FnType type = FnType::FUNCTION;
		sptr<Token> first;
		sptr<Token> name;
		std::vector<sptr<Token>> invokeDims;
		std::vector<uptr<ParsedFnArg>> args;
		sptr<ParsedType> returnType;
		sptr<GenericSignature> genSig;
		uptr<ScopeStatement> code;

	};

	struct FunctionImpl
	{
		const ptr<Function> parent;
		const sptr<GenericArguments> genArgs;

		FunctionImpl(ptr<Function> p, sptr<GenericArguments> gArgs) : parent(p), genArgs(gArgs) {}
		virtual ~FunctionImpl() {}

		cllr::TypedSSA emitFnDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm);

		/*
		Making functions responsible for emitting call instructions means we can override this behavior and implement some free inlining
		*/
		virtual cllr::TypedSSA call(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, in<std::vector<cllr::TypedSSA>> args, sptr<Token> callTkn = nullptr);

	private:
		sptr<SymbolTable> fnImplTable = nullptr;
		cllr::SSA id = 0;
		cllr::SSA retTypeID = 0;

	};

	struct MethodImpl : FunctionImpl
	{

	};

	struct Function : Generic<FunctionImpl>
	{
		const FnType type;

		sptr<Token> first;
		sptr<Token> name;
		std::vector<sptr<Token>> invokeDims;
		std::vector<uptr<ParsedFnArg>> args;
		sptr<ParsedType> returnType;
		sptr<GenericSignature> genSig;
		uptr<ScopeStatement> code;

		Function(ref<ParsedFn> fn) : Generic(fn.genSig, lambda_v(sptr<GenericArguments> gArgs){
			return new_sptr<FunctionImpl>(this, gArgs);
		}), type(fn.type) {
			first = fn.first;
			name = fn.name;
			invokeDims = fn.invokeDims;
			args = std::move(fn.args);
			returnType = fn.returnType;
			genSig = fn.genSig;
			code = std::move(fn.code);
		}
		virtual ~Function() {}

	};

	struct FunctionGroup
	{
		std::vector<sptr<Function>> unresolvedFns;
		std::vector<std::pair<std::vector<cllr::SSA>, sptr<Function>>> fns;

		FunctionGroup() = default;
		virtual ~FunctionGroup() {}

		void resolve(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) {}

		sptr<Function> find(in<std::vector<cllr::TypedSSA>> args, in<cllr::Assembler> codeAsm);

		void add(sptr<Function> fn)
		{
			unresolvedFns.push_back(fn);
		}

	};
	
	struct Method : Function
	{
		Method(ref<ParsedFn> fnData) : Function(fnData)
		{

		}
		/*
		sptr<FunctionImpl> makeVariant(sptr<GenericArguments> args) override
		{

		}
		*/
	};

	struct Constructor : Method
	{

	};

	struct Destructor : Method
	{

	};

}
