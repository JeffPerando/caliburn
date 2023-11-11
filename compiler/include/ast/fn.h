
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
		std::vector<uptr<ParsedFnArg>> args;
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

		Function(in<std::string> n) : name(n) {}
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
		std::vector<uptr<ParsedFnArg>> args;
		sptr<ParsedType> returnType;
		uptr<GenericSignature> genSig;
		uptr<ScopeStmt> code;

		UserFn(ref<ParsedFn> fn) : Function(name->str), type(fn.type) {
			first = fn.first;
			name = fn.name;
			invokeDims = fn.invokeDims;
			args = std::move(fn.args);
			returnType = fn.returnType;
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

	using BnFnImplLambda = std::function<cllr::TypedSSA(sptr<const SymbolTable>, out<cllr::Assembler>, in<std::vector<cllr::TypedSSA>>)>;
	
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
		const BnFnImplLambda fnImpl;
		const uptr<GenericSignature> genSig;

		GenArgMap<BuiltinFnImpl> genericImpls;

		BuiltinFn(in<std::string> name, in<BnFnImplLambda> impl) : Function(name), fnImpl(impl) {}
		BuiltinFn(in<std::string> name, in<BnFnImplLambda> impl, out<uptr<GenericSignature>> gSig) : Function(name), fnImpl(impl),
			genSig(gSig != nullptr ? std::move(gSig) : nullptr) {}
		virtual ~BuiltinFn() = default;

		sptr<FnImpl> getImpl(sptr<GenericArguments> gArgs) override;

	};

	struct FunctionGroup
	{
		std::vector<sptr<Function>> unresolvedFns;
		std::vector<std::pair<std::vector<sptr<cllr::LowType>>, sptr<Function>>> fns;

		FunctionGroup() = default;
		virtual ~FunctionGroup() = default;

		void resolve(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) {}

		sptr<Function> find(in<std::vector<cllr::TypedSSA>> args, in<cllr::Assembler> codeAsm);

		void add(sptr<Function> fn)
		{
			unresolvedFns.push_back(fn);
		}

	};

	struct MethodImpl : UserFnImpl
	{

	};

	struct Method : UserFn
	{
		Method(ref<ParsedFn> fnData) : UserFn(fnData)
		{

		}

	};

	struct Constructor : Method
	{

	};

	struct Destructor : Method
	{

	};

}
