
#include "ast/scopestmt.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

void ScopeStmt::prettyPrint(out<std::stringstream> ss) const
{
	ss << "{\n";

	for (auto const& stmt : stmts)
	{
		stmt->prettyPrint(ss);
		ss << ';\n';
	}

	ss << "}";

}

void ScopeStmt::declareSymbols(sptr<SymbolTable> table)
{
	if (scopeTable != nullptr)
	{
		return;
	}

	scopeTable = new_sptr<SymbolTable>(table);

	for (auto const& stmt : stmts)
	{
		stmt->declareSymbols(scopeTable);

	}

}

void ScopeStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	scopeTable->reparent(table);

	for (auto const& inner : stmts)
	{
		inner->emitCodeCLLR(scopeTable, codeAsm);
	}

}

void BreakStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { codeAsm.getLoopEnd() }));
}

void ContinueStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { codeAsm.getLoopStart() }));
}

void DiscardStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	codeAsm.push(cllr::Instruction(cllr::Opcode::DISCARD));
}

void ReturnStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (retValue != nullptr)
	{
		MATCH(retValue->emitValueCLLR(table, codeAsm), cllr::TypedSSA, ret)
		{
			codeAsm.push(cllr::Instruction(cllr::Opcode::RETURN_VALUE, {}, { ret->value }));

		}
	else
	{
		codeAsm.errors->err("Invalid return value", *retValue);
	}

	}
	else
	{
		codeAsm.push(cllr::Instruction(cllr::Opcode::RETURN));
	}

}

void PassStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	//TODO implement
}

void UnreachableStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	//TODO implement
}
