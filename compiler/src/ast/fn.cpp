
#include "ast/fn.h"

using namespace caliburn;

cllr::SSA FunctionImpl::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return id;
	}

	auto& sig = parent->sig;
	auto& genSig = sig->genSig;
	auto& retTypeP = sig->returnType;

	if (fnImplTable == nullptr)
	{
		fnImplTable = new_sptr<SymbolTable>();

		genArgs->apply(genSig, fnImplTable);

	}

	fnImplTable->reparent(table);

	auto retType = retTypeP->resolve(fnImplTable);
	auto retTID = retType->emitDeclCLLR(table, codeAsm);

	id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::FUNCTION, { (uint32_t)sig->args.size() }, { retTID }));

	for (auto const& arg : sig->args)
	{
		arg->emitDeclCLLR(fnImplTable, codeAsm);
	}

	parent->code->emitDeclCLLR(fnImplTable, codeAsm);

	codeAsm.push(cllr::Instruction(cllr::Opcode::FUNCTION_END, {}, { id }));

	return id;
}

cllr::TypedSSA FunctionImpl::call(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, ref<const std::vector<sptr<Value>>> args)
{
	auto fnID = emitDeclCLLR(table, codeAsm);

	fnImplTable->reparent(table);

	if (auto t = parent->sig->returnType->resolve(fnImplTable))
	{
		auto tID = t->emitDeclCLLR(fnImplTable, codeAsm);
		auto callID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::CALL, { (uint32_t)args.size() }, { fnID }));

		uint32_t index = 0;

		for (auto& arg : args)
		{
			auto argID = arg->emitValueCLLR(fnImplTable, codeAsm).value;

			codeAsm.push(cllr::Instruction(cllr::Opcode::CALL_ARG, { index }, { argID }));

			++index;

		}

		return cllr::TypedSSA(t, tID, callID);
	}

	//TODO complain
	return cllr::TypedSSA();
}
