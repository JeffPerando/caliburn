
#include "ast/fn.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

cllr::TypedSSA FunctionImpl::emitFnDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return cllr::TypedSSA(retType, id);
	}

	auto& args = parent->args;
	auto& genSig = parent->genSig;
	auto& pRetType = parent->returnType;

	if (fnImplTable == nullptr)
	{
		fnImplTable = new_sptr<SymbolTable>();
	}

	genArgs->apply(genSig, fnImplTable);
	fnImplTable->reparent(table);

	auto retType = pRetType->resolve(fnImplTable);

	if (retType == nullptr)
	{
		auto e = codeAsm.errors->err({ "Could not resolve return type", pRetType->name }, *pRetType);

		return cllr::TypedSSA();
	}

	auto retTypeImpl = retType->emitTypeCLLR(fnImplTable, codeAsm);

	id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::FUNCTION, { (uint32_t)args.size() }, { retTypeImpl->id }));

	for (auto i = 0; i < args.size(); ++i)
	{
		auto const& arg = args[i];
		auto fnArg = new_sptr<FnArgVariable>(*arg, i);

		fnImplTable->add(fnArg->nameTkn->str, fnArg);

		fnArg->emitDeclCLLR(fnImplTable, codeAsm);

	}

	parent->code->emitDeclCLLR(fnImplTable, codeAsm);

	codeAsm.push(cllr::Instruction(cllr::Opcode::FUNCTION_END, {}, { id }));

	return cllr::TypedSSA(retTypeImpl, id);
}

/*
Making functions responsible for emitting call instructions means we can override this behavior and implement some free inlining
*/
cllr::TypedSSA FunctionImpl::call(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, in<std::vector<cllr::TypedSSA>> args, sptr<Token> callTkn)
{
	auto fnData = emitFnDeclCLLR(table, codeAsm);

	auto callID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::CALL, { (uint32_t)args.size() }, { fnData.value }, fnData.type->id).debug(callTkn));

	for (uint32_t i = 0; i < args.size(); ++i)
	{
		codeAsm.push(cllr::Instruction(cllr::Opcode::CALL_ARG, { i }, { args[i].value }));
	}

	return cllr::TypedSSA(fnData.type, callID);
}

sptr<Function> FunctionGroup::find(in<std::vector<cllr::TypedSSA>> args, in<cllr::Assembler> codeAsm)
{
	//TODO score-based system; fewer conversions = higher score, highest-scording function gets returned

	cllr::TypeChecker checker(codeAsm.settings);
	/*
	for (auto const& [types, fn] : fns)
	{
		if (types.size() != args.size())
		{
			continue;
		}

		bool valid = true;

		for (size_t i = 0; i < args.size(); ++i)
		{
			cllr::SSA t = types[i];
			auto& val = args[i];

			if (checker.lookup(t, val, codeAsm) == cllr::ConvertResult::INCOMPATIBLE)
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
	*/
	return nullptr;
}
