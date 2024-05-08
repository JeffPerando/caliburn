
#include "ast/fn.h"
#include "ast/values.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

cllr::TypedSSA SrcFn::call(in<std::vector<cllr::TypedSSA>> callIDs, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm)
{
	if (genSig != nullptr)
	{
		if (gArgs == nullptr || gArgs->empty())
		{
			gArgs = genSig->makeDefaultArgs();
		}

		if (!genSig->canApply(*gArgs))
		{
			//TODO complain
			return cllr::TypedSSA();
		}

	}
	
	sptr<SrcFnImpl> impl = nullptr;

	if (auto found = variants.find(gArgs); found != variants.end())
	{
		impl = found->second;
	}
	else
	{
		auto table = makeFnContext(callIDs, gArgs, codeAsm);
		
		impl = new_sptr<SrcFnImpl>(table, args, retType, code);
		variants.insert(std::pair(gArgs, impl));
	}

	return impl->call(callIDs, codeAsm);
}

cllr::TypedSSA SrcFnImpl::emitFnDeclCLLR(out<cllr::Assembler> codeAsm)
{
	auto rt = (retType == nullptr ?
		codeAsm.pushType(cllr::Opcode::TYPE_VOID)
		: retType->resolve(syms, codeAsm));

	if (rt == nullptr)
	{
		auto e = codeAsm.errors->err({ "Could not resolve return type", retType->name }, *retType);

		return cllr::TypedSSA();
	}

	if (id != 0)
	{
		return cllr::TypedSSA(rt, id);
	}

	id = codeAsm.beginSect(cllr::Instruction(cllr::Opcode::FUNCTION, { (uint32_t)args->size() }, { rt->id }));

	for (auto i = 0; i < args->size(); ++i)
	{
		auto fnArg = new_sptr<FnArgVariable>((*args)[i], i);

		syms->add(fnArg->nameTkn->str, fnArg);

		fnArg->emitVarCLLR(syms, codeAsm);

	}

	code->emitCodeCLLR(syms, codeAsm);

	codeAsm.endSect(cllr::Instruction(cllr::Opcode::FUNCTION_END, {}, { id }));

	return cllr::TypedSSA(rt, id);
}

cllr::TypedSSA SrcFnImpl::call(in<std::vector<cllr::TypedSSA>> callArgs, out<cllr::Assembler> codeAsm)
{
	auto fnData = emitFnDeclCLLR(codeAsm);

	auto call = codeAsm.pushValue(cllr::Instruction(cllr::Opcode::CALL, { (uint32_t)callArgs.size() }, { id }), fnData.type);

	for (uint32_t i = 0; i < callArgs.size(); ++i)
	{
		codeAsm.push(cllr::Instruction(cllr::Opcode::CALL_ARG, { i }, { callArgs[i].value }));
	}

	return call;
}

cllr::TypedSSA FunctionGroup::call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm)
{
	//TODO score-based system; fewer conversions = higher score, highest-scording function gets returned

	cllr::TypeChecker checker(codeAsm.settings);

	for (in<sptr<Function>> fn : fns)
	{
		if (fn->args->size() != args.size())
		{
			continue;
		}

		bool valid = true;

		auto const& fnArgs = *fn->args;

		for (size_t i = 0; i < args.size(); ++i)
		{
			auto const& fnArg = fnArgs[i];
			auto t = fnArg.typeHint->resolve(nullptr, codeAsm);
			auto& val = args[i];

			if (checker.lookup(t, val, codeAsm) == cllr::TypeCheckResult::INCOMPATIBLE)
			{
				valid = false;
				break;
			}

		}

		if (valid)
		{
			return fn->call(args, gArgs, codeAsm);
		}

	}

	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA BuiltinFn::call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm)
{
	auto table = makeFnContext(args, gArgs, codeAsm);
	auto rt = retType->resolve(table, codeAsm);

	return fnImpl(table, codeAsm, args, rt);
}

sptr<SymbolTable> Function::makeFnContext(in<std::vector<cllr::TypedSSA>> callIDs, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm)
{
	auto table = new_sptr<SymbolTable>();

	if (genSig != nullptr && gArgs != nullptr)
	{
		gArgs->apply(*genSig, table, codeAsm);

	}

	return table;
}

sptr<SymbolTable> Method::makeFnContext(in<std::vector<cllr::TypedSSA>> callIDs, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm)
{
	auto table = Function::makeFnContext(callIDs, gArgs, codeAsm);
	auto self = new_sptr<VarReadValue>("this");
	auto mems = callIDs[0].type->getMembers();

	for (auto& mem : mems)
	{
		if (!table->add(mem, new_sptr<MemberReadDirectValue>(self, mem)))
		{
			codeAsm.errors->err({ "Duplicate name:", mem }, nullptr);

		}

	}

	return table;
}

cllr::TypedSSA SrcMethod::call(in<std::vector<cllr::TypedSSA>> argVals, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm)
{
	if (gArgs == nullptr || gArgs->empty())
	{
		gArgs = genSig->makeDefaultArgs();
	}

	if (!genSig->canApply(*gArgs))
	{
		//TODO complain
		return cllr::TypedSSA();
	}

	sptr<SrcFnImpl> impl = nullptr;

	if (auto found = variants.find(gArgs); found != variants.end())
	{
		impl = found->second;
	}
	else
	{
		auto table = makeFnContext(argVals, gArgs, codeAsm);
		
		impl = new_sptr<SrcFnImpl>(table, args, retType, code);
		variants.insert(std::pair(gArgs, impl));
	}

	return impl->call(argVals, codeAsm);
}

cllr::TypedSSA BuiltinMethod::call(in<std::vector<cllr::TypedSSA>> args, sptr<GenericArguments> gArgs, out<cllr::Assembler> codeAsm)
{
	auto table = makeFnContext(args, gArgs, codeAsm);
	auto rt = retType->resolve(table, codeAsm);

	return fnImpl(table, codeAsm, args, rt);
}
