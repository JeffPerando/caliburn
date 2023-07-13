
#include "ast/fn.h"

#include "ast/ast.h"

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

	sptr<SymbolTable> fnTable = table;

	if (!genSig->empty())
	{
		fnTable = new_sptr<SymbolTable>(table);
		genArgs->apply(genSig, fnTable);
	}

	auto retType = retTypeP->resolve(fnTable);
	auto retTID = retType->emitDeclCLLR(table, codeAsm);

	id = codeAsm.pushNew(cllr::Opcode::FUNCTION, { (uint32_t)sig->args.size() }, { retTID });

	for (auto const& arg : sig->args)
	{
		arg->emitDeclCLLR(fnTable, codeAsm);
	}

	parent->code->emitDeclCLLR(fnTable, codeAsm);

	codeAsm.push(0, cllr::Opcode::FUNCTION_END, {}, { id });

	return id;
}

cllr::TypedSSA FunctionImpl::call(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, ref<const std::vector<sptr<Value>>> args)
{
	auto fnID = emitDeclCLLR(table, codeAsm);

	auto tID = codeAsm.codeFor(fnID)->refs[0];

	auto callID = codeAsm.pushNew(cllr::Opcode::CALL, { (uint32_t)args.size() }, { fnID });

	uint32_t index = 0;

	for (auto& arg : args)
	{
		auto argID = arg->emitValueCLLR(table, codeAsm).value;

		codeAsm.push(0, cllr::Opcode::CALL_ARG, { index }, { argID });

		++index;

	}

	return cllr::TypedSSA(parent->sig->returnType->resolve(table), callID, tID);
}
