
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
		uptr<ScopeStmt> code;

	};

	struct FnImpl
	{
		FnImpl() = default;
		virtual ~FnImpl() = default;

		/*
		Making functions responsible for emitting call instructions means we can override this behavior and implement some free inlining
		*/
		virtual cllr::TypedSSA call(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, in<std::vector<cllr::TypedSSA>> args, sptr<Token> callTkn = nullptr) = 0;

	};

	struct Function
	{
		const std::string name;
		const std::vector<FnArg> args;
		const sptr<ParsedType> retType;

		Function(in<std::string> n, in<std::vector<FnArg>> as, sptr<ParsedType> rt) : name(n), args(as), retType(rt) {}
		virtual ~Function() = default;

		virtual sptr<FnImpl> getImpl(sptr<GenericArguments> gArgs) = 0;

	};

	struct UserFnImpl;

	struct UserFn : Function
	{
	private:
		GenArgMap<UserFnImpl> variants;
	public:
		const FnType type;

		sptr<Token> first;
		sptr<Token> name;
		std::vector<sptr<Token>> invokeDims;
		uptr<GenericSignature> genSig;
		uptr<ScopeStmt> code;

		UserFn(ref<ParsedFn> fn) : Function(name->str, fn.args, fn.returnType), type(fn.type) {
			first = fn.first;
			name = fn.name;
			invokeDims = fn.invokeDims;
			genSig = std::move(fn.genSig);
			code = std::move(fn.code);
		}
		virtual ~UserFn() = default;

		sptr<FnImpl> getImpl(sptr<GenericArguments> gArgs) override;

	};

	struct UserFnImpl : FnImpl
	{
		const ptr<UserFn> parent;
		const sptr<GenericArguments> genArgs;

		UserFnImpl(ptr<UserFn> p, sptr<GenericArguments> gArgs) : parent(p), genArgs(gArgs) {}
		virtual ~UserFnImpl() = default;

		cllr::TypedSSA emitFnDeclCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm);

		/*
		Making functions responsible for emitting call instructions means we can override this behavior and implement some free inlining
		*/
		cllr::TypedSSA call(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, in<std::vector<cllr::TypedSSA>> args, sptr<Token> callTkn = nullptr) override;

	private:
		sptr<SymbolTable> fnImplTable = nullptr;
		cllr::SSA id = 0;
		sptr<cllr::LowType> retType = nullptr;

	};

	using BnFnImplLambda = std::function<cllr::TypedSSA(sptr<const SymbolTable>, out<cllr::Assembler>, in<std::vector<cllr::TypedSSA>>, sptr<cllr::LowType> outType)>;
	
	struct BuiltinFn;

	struct BuiltinFnImpl : FnImpl
	{
		const ptr<BuiltinFn> parent;
		const sptr<GenericArguments> genArgs;

		BuiltinFnImpl(sptr<GenericArguments> gArgs, ptr<BuiltinFn> p) : genArgs(gArgs), parent(p) {};
		virtual ~BuiltinFnImpl() = default;

		cllr::TypedSSA call(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, in<std::vector<cllr::TypedSSA>> args, sptr<Token> callTkn = nullptr) override;

	};

	struct BuiltinFn : Function
	{
	public:
		const BnFnImplLambda fnImpl;
		const uptr<GenericSignature> genSig;
		
		GenArgMap<BuiltinFnImpl> genericImpls;

		BuiltinFn(in<std::string> name, in<std::vector<FnArg>> args, sptr<ParsedType> rt, in<BnFnImplLambda> impl) :
			Function(name, args, rt), fnImpl(impl), genSig(nullptr) {}

		BuiltinFn(in<std::string> name, in<std::vector<FnArg>> args, sptr<ParsedType> rt, in<BnFnImplLambda> impl, in<GenericSignature> gSig) :
			Function(name, args, rt), fnImpl(impl), genSig(new_uptr<GenericSignature>(gSig)) {}

		virtual ~BuiltinFn() = default;

		sptr<FnImpl> getImpl(sptr<GenericArguments> gArgs) override;

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

		sptr<Function> find(in<std::vector<cllr::TypedSSA>> args, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm);

		void add(sptr<Function> fn)
		{
			fns.push_back(fn);
		}

	};

	struct MethodImpl : UserFnImpl
	{

	};

	struct Method : UserFn
	{
		Method(ref<ParsedFn> fnData) : UserFn(fnData) {}

	};

	struct Constructor : Method
	{

	};

	struct Destructor : Method
	{

	};

}
