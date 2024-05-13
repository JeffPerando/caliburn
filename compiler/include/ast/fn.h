
#pragma once

#include <functional>
#include <map>

#include "basic.h"
#include "langcore.h"

#include "ast/ast.h"
#include "ast/generics.h"
#include "ast/scopestmt.h"
#include "ast/symbols.h"
#include "ast/type.h"
#include "ast/var.h"

#include "cllr/cllrasm.h"

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
		{"new", FnType::CONSTRUCTOR},
		{"delete", FnType::DESTRUCTOR},
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
		virtual sptr<SymbolTable> makeFnContext(in<std::vector<cllr::TypedSSA>> callIDs, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm);

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

		SrcFn(in<std::string> n, out<uptr<GenericSignature>> gSig, in<std::vector<FnArg>> as, sptr<ParsedType> rt, sptr<ScopeStmt> impl) :
			Function(n, gSig, as, rt),
			code(impl) {}

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
		const sptr<SymbolTable> genTable;

		Method(sptr<ParsedType> me, sptr<SymbolTable> gt, in<std::string> n, out<uptr<GenericSignature>> gSig, in<std::vector<FnArg>> as, sptr<ParsedType> rt) :
			Function(n, gSig, as, rt),
			self(me), genTable(gt)
		{
			args->insert(args->begin(), FnArg{ me, "this" });
		}

		virtual ~Method() = default;

		sptr<SymbolTable> makeFnContext(in<std::vector<cllr::TypedSSA>> callIDs, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) override;

	};

	struct SrcMethod : Method
	{
	private:
		GenArgMap<SrcFnImpl> variants;
	public:
		std::vector<sptr<Token>> invokeDims;
		sptr<ScopeStmt> code;

		SrcMethod(sptr<ParsedType> me, sptr<SymbolTable> gt, out<ParsedFn> fn) :
			Method(me, gt, fn.name->str, fn.genSig, fn.args, fn.returnType),
			invokeDims(fn.invokeDims), code(fn.code) {}

		SrcMethod(sptr<ParsedType> me, sptr<SymbolTable> gt, in<std::string> n, out<uptr<GenericSignature>> gSig, in<std::vector<FnArg>> as, sptr<ParsedType> rt, sptr<ScopeStmt> impl) :
			Method(me, gt, n, gSig, as, rt),
			code(impl) {}

		virtual ~SrcMethod() = default;

		cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) override;

	};

	struct BuiltinMethod : Method
	{
	private:
		const BnFnImplLambda fnImpl;

	public:
		BuiltinMethod(sptr<ParsedType> me, sptr<SymbolTable> gt, in<std::string> name, out<uptr<GenericSignature>> gSig, in<std::vector<FnArg>> args, sptr<ParsedType> rt, in<BnFnImplLambda> impl) :
			Method(me, gt, name, gSig, args, rt), fnImpl(impl) {}

		virtual ~BuiltinMethod() = default;

		cllr::TypedSSA call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm) override;

	};

	struct SrcCtor : SrcFn
	{
		SrcCtor(sptr<ParsedType> me, sptr<SymbolTable> gt, out<ParsedFn> fn)
			: SrcFn(me->prettyStr().append("_new"), SCAST<uptr<GenericSignature>>(nullptr), fn.args, me, fn.code) {}

		virtual ~SrcCtor() = default;

	};

	struct BuiltinCtor : BuiltinFn
	{
		BuiltinCtor(sptr<ParsedType> me, sptr<SymbolTable> gt, in<std::vector<FnArg>> args, in<BnFnImplLambda> impl) :
			BuiltinFn(me->prettyStr().append("_new"), SCAST<uptr<GenericSignature>>(nullptr), args, me, impl) {}

		virtual ~BuiltinCtor() = default;

	};

	struct SrcDtor : SrcMethod
	{
		SrcDtor(sptr<ParsedType> me, sptr<SymbolTable> gt, out<ParsedFn> fn) :
			SrcMethod(me, gt, me->prettyStr().append("_delete"), SCAST<uptr<GenericSignature>>(nullptr), {}, new_sptr<ParsedType>("void"), fn.code) {}

		virtual ~SrcDtor() = default;

	};

	struct BuiltinDtor : BuiltinMethod
	{
		BuiltinDtor(sptr<ParsedType> me, sptr<SymbolTable> gt, in<BnFnImplLambda> impl) :
			BuiltinMethod(me, gt, me->prettyStr().append("_delete"), SCAST<uptr<GenericSignature>>(nullptr), {}, new_sptr<ParsedType>("void"), impl) {}

		virtual ~BuiltinDtor() = default;

	};

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
