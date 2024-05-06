
#pragma once

#include <functional>
#include <map>

#include "basic.h"
#include "langcore.h"

#include "ast.h"
#include "generics.h"
#include "scopestmt.h"
#include "symbols.h"
#include "var.h"

#include "cllr/cllrasm.h"

#include "types/type.h"

namespace caliburn
{
	namespace cllr
	{
		struct LowType;
	}

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
		std::vector<FnArg> args;
		sptr<ParsedType> returnType;
		uptr<GenericSignature> genSig;
		sptr<ScopeStmt> code;

	};

	struct Function
	{
		const std::string name;
		const uptr<GenericSignature> genSig;
		const sptr<std::vector<FnArg>> args;
		const sptr<ParsedType> retType;

		Function(in<std::string> n, out<uptr<GenericSignature>> gSig, in<std::vector<FnArg>> as, sptr<ParsedType> rt) :
			name(n), genSig(std::move(gSig)), args(new_sptr<std::vector<FnArg>>(as)), retType(rt) {}

		virtual ~Function() = default;

		virtual cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) = 0;

	protected:
		virtual sptr<SymbolTable> makeFnContext(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm);

	};

	struct SrcFnImpl
	{
		cllr::SSA id = 0;
		sptr<SymbolTable> syms;
		sptr<std::vector<FnArg>> args;
		sptr<ParsedType> retType;
		sptr<ScopeStmt> code;

		SrcFnImpl(sptr<SymbolTable> cxt, sptr<std::vector<FnArg>> as, sptr<ParsedType> rt, sptr<ScopeStmt> impl) :
			syms(cxt), args(as), retType(rt), code(impl) {}
		virtual ~SrcFnImpl() = default;

		cllr::TypedSSA emitFnDeclCLLR(out<cllr::Assembler> codeAsm);

		cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, out<cllr::Assembler> codeAsm);

	};

	struct SrcFn : Function
	{
	private:
		GenArgMap<SrcFnImpl> variants;
	public:
		std::vector<sptr<Token>> invokeDims;
		sptr<ScopeStmt> code;

		SrcFn(out<ParsedFn> fn) :
			Function(fn.name->str, fn.genSig, fn.args, fn.returnType),
			invokeDims(fn.invokeDims), code(fn.code) {}

		virtual ~SrcFn() = default;

		cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) override;

	};

	using BnFnImplLambda = std::function<cllr::TypedSSA(sptr<const SymbolTable>, out<cllr::Assembler>, in<std::vector<cllr::TypedSSA>>, sptr<cllr::LowType> outType)>;

	struct BuiltinFn : Function
	{
	private:
		const BnFnImplLambda fnImpl;

	public:
		BuiltinFn(in<std::string> name, out<uptr<GenericSignature>> gSig, in<std::vector<FnArg>> args, sptr<ParsedType> rt, in<BnFnImplLambda> impl) :
			Function(name, gSig, args, rt), fnImpl(impl) {}

		virtual ~BuiltinFn() = default;

		cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) override;

	};

	struct Method : Function
	{
		const sptr<ParsedType> self;

		Method(sptr<ParsedType> me, in<std::string> n, out<uptr<GenericSignature>> gSig, in<std::vector<FnArg>> as, sptr<ParsedType> rt) :
			Function(n, gSig, as, rt),
			self(me)
		{
			args->insert(args->begin(), FnArg{ me, "this" });
		}

		virtual ~Method() = default;

		sptr<SymbolTable> makeFnContext(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) override;

	};

	struct SrcMethod : Method
	{
	private:
		GenArgMap<SrcFnImpl> variants;
	public:
		std::vector<sptr<Token>> invokeDims;
		sptr<ScopeStmt> code;

		SrcMethod(sptr<ParsedType> me, ref<ParsedFn> fn) :
			Method(me, fn.name->str, fn.genSig, fn.args, fn.returnType),
			invokeDims(fn.invokeDims), code(fn.code) {}

		virtual ~SrcMethod() = default;

		cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) override;

	};

	struct BuiltinMethod : Method
	{
	private:
		const BnFnImplLambda fnImpl;

	public:
		BuiltinMethod(sptr<ParsedType> me, in<std::string> name, out<uptr<GenericSignature>> gSig, in<std::vector<FnArg>> args, sptr<ParsedType> rt, in<BnFnImplLambda> impl) :
			Method(me, name, gSig, args, rt), fnImpl(impl) {}

		virtual ~BuiltinMethod() = default;

		cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) override;

	};

	/*
	struct Constructor : Method
	{

	};

	struct Destructor : Method
	{

	};
	*/

	struct FunctionGroup
	{
		std::vector<sptr<Function>> fns;

		FunctionGroup() = default;
		FunctionGroup(sptr<Function> fn)
		{
			fns.push_back(fn);
		}
		virtual ~FunctionGroup() = default;

		virtual cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm);

		virtual void add(sptr<Function> fn)
		{
			fns.push_back(fn);
		}

	};

}
