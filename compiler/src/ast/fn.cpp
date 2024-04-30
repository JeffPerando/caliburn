
#include "ast/fn.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

sptr<FnImpl> UserFn::getImpl(sptr<GenericArguments> gArgs)
{
	if (!genSig->canApply(*gArgs))
	{
		return nullptr;
	}

	if (gArgs->empty())
	{
		gArgs = genSig->makeDefaultArgs();
	}

	if (auto found = variants.find(gArgs); found != variants.end())
	{
		return found->second;
	}

	auto impl = new_sptr<UserFnImpl>(this, gArgs);

	variants.insert(std::pair(gArgs, impl));

	return impl;
}

cllr::TypedSSA UserFnImpl::emitFnDeclCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return cllr::TypedSSA(retType, id);
	}

	auto& code = parent->code;
	auto& args = parent->args;
	auto& genSig = parent->genSig;
	auto& pRetType = parent->retType;

	if (fnImplTable == nullptr)
	{
		fnImplTable = new_sptr<SymbolTable>();
	}

	genArgs->apply(*genSig, fnImplTable, codeAsm);
	fnImplTable->reparent(table);

	auto retType = pRetType->resolve(fnImplTable, codeAsm);

	if (retType == nullptr)
	{
		auto e = codeAsm.errors->err({ "Could not resolve return type", pRetType->name }, *pRetType);

		return cllr::TypedSSA();
	}

	id = codeAsm.beginSect(cllr::Instruction(cllr::Opcode::FUNCTION, { (uint32_t)args.size() }, { retType->id }));

	for (auto i = 0; i < args.size(); ++i)
	{
		auto fnArg = new_sptr<FnArgVariable>(args[i], i);

		fnImplTable->add(fnArg->nameTkn->str, fnArg);

		fnArg->emitVarCLLR(fnImplTable, codeAsm);

	}

	//TODO does this do anything?
	//code->declareSymbols(fnImplTable);
	code->emitCodeCLLR(fnImplTable, codeAsm);

	codeAsm.endSect(cllr::Instruction(cllr::Opcode::FUNCTION_END, {}, { id }));

	return cllr::TypedSSA(retType, id);
}

/*
Making functions responsible for emitting call instructions means we can override this behavior and implement some free inlining
*/
cllr::TypedSSA UserFnImpl::call(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, in<std::vector<cllr::TypedSSA>> args, sptr<Token> callTkn)
{
	auto fnData = emitFnDeclCLLR(table, codeAsm);

	auto callID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::CALL, { (uint32_t)args.size() }, { fnData.value }, fnData.type->id).debug(callTkn));

	for (uint32_t i = 0; i < args.size(); ++i)
	{
		codeAsm.push(cllr::Instruction(cllr::Opcode::CALL_ARG, { i }, { args[i].value }));
	}

	return cllr::TypedSSA(fnData.type, callID);
}

sptr<Function> FunctionGroup::find(in<std::vector<cllr::TypedSSA>> args, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	//TODO score-based system; fewer conversions = higher score, highest-scording function gets returned

	cllr::TypeChecker checker(codeAsm.settings);
	
	for (auto const& fn : fns)
	{
		if (fn->args.size() != args.size())
		{
			continue;
		}
		
		bool valid = true;

		for (size_t i = 0; i < args.size(); ++i)
		{
			auto const& fnArg = fn->args[i];

			auto t = fnArg.typeHint->resolve(table, codeAsm);

			auto& val = args[i];

			if (checker.lookup(t, val, codeAsm) == cllr::TypeCheckResult::INCOMPATIBLE)
			{
				valid = false;
				break;
			}

		}

		if (valid)
		{
			return fn;
		}

	}
	
	return nullptr;
}

sptr<FnImpl> BuiltinFn::getImpl(sptr<GenericArguments> gArgs)
{
	auto found = genericImpls.find(gArgs);

	if (found != genericImpls.end())
	{
		return found->second;
	}

	auto impl = new_sptr<BuiltinFnImpl>(gArgs, this);

	genericImpls[gArgs] = impl;

	return impl;
}

cllr::TypedSSA BuiltinFnImpl::call(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, in<std::vector<cllr::TypedSSA>> args, sptr<Token> callTkn)
{
	auto retType = this->parent->retType->resolve(table, codeAsm);

	if (retType == nullptr)
	{
		//TODO complain
	}

	if (!genArgs->empty())
	{
		auto genSyms = new_sptr<SymbolTable>(table);

		genArgs->apply(*parent->genSig, genSyms, codeAsm);

		return parent->fnImpl(genSyms, codeAsm, args, retType);
	}

	return parent->fnImpl(table, codeAsm, args, retType);
}
