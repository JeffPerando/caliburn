
#include "ast/setstmt.h"
#include "cllr/cllrtype.h"

using namespace caliburn;

ValueResult SetStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto lres = lhs->emitCodeCLLR(table, codeAsm);
	auto rres = rhs->emitCodeCLLR(table, codeAsm);

	cllr::TypedSSA lval, rval;

	MATCH(lres, cllr::TypedSSA, l)
	{
		lval = *l;
	}
	else
	{
		//TODO complain
		return ValueResult();
	}

	MATCH(rres, cllr::TypedSSA, r)
	{
		rval = *r;
	}
	else
	{
		//TODO complain
		return ValueResult();
	}

	cllr::TypeChecker tc(codeAsm.settings);

	cllr::TypedSSA result;
	if (!tc.check(lval.type, rval, result, codeAsm, Operator::NONE))
	{
		//TODO complain
		return ValueResult();
	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { lval.value, result.value }));

	return ValueResult();
}
