
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
		sptr<GenericSignature> genSig;
		std::vector<uptr<ParsedFnArg>> args;
		sptr<ParsedType> retType;
		uptr<ScopeStatement> code;

	};

	struct FunctionSignature
	{
		std::vector<sptr<FnArgVariable>> args;
		sptr<GenericSignature> genSig;
		sptr<ParsedType> returnType;

		FunctionSignature() = default;
		FunctionSignature(ref<const ParsedFn> fn) : genSig(fn.genSig), returnType(fn.retType)
		{
			uint32_t a = 0;
			for (auto const& pFnArg : fn.args)
			{
				auto const& [type, name] = *pFnArg;

				auto arg = new_sptr<FnArgVariable>(a);

				arg->typeHint = type;
				arg->nameTkn = name;

				args.push_back(arg);
				++a;

			}

		}

	};

	struct FunctionImpl : cllr::Emitter
	{
		const ptr<Function> parent;
		const sptr<GenericArguments> genArgs;

		FunctionImpl(ptr<Function> p, sptr<GenericArguments> gArgs) : parent(p), genArgs(gArgs) {}
		virtual ~FunctionImpl() {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

		virtual cllr::TypedSSA call(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, ref<const std::vector<sptr<Value>>> args);

	private:
		sptr<SymbolTable> fnImplTable = nullptr;
		cllr::SSA id = 0;

	};

	struct MethodImpl : FunctionImpl
	{

	};

	struct Function : Generic<FunctionImpl>
	{
		const sptr<FunctionSignature> sig;

		sptr<Token> name = nullptr;
		uptr<ScopeStatement> code = nullptr;

		Function(ref<ParsedFn> fn) : Function(new_sptr<FunctionSignature>(fn))
		{
			name = fn.name;
			code = std::move(fn.code);
		}
		Function(sptr<FunctionSignature> sig) : Generic(sig->genSig, lambda_v(sptr<GenericArguments> gArgs){
			return new_sptr<FunctionImpl>(this, gArgs);
		}), sig(sig) {}
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
