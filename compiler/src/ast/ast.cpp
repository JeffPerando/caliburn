
#include "ast/ast.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

void ScopeStatement::prettyPrint(out<std::stringstream> ss) const
{
	ss << "{\n";

	for (auto const& stmt : stmts)
	{
		stmt->prettyPrint(ss);
		ss << ';\n';
	}

	ss << "}";

}

void ScopeStatement::declareSymbols(sptr<SymbolTable> table)
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

cllr::SSA ScopeStatement::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	scopeTable->reparent(table);

	for (auto const& inner : stmts)
	{
		inner->emitDeclCLLR(scopeTable, codeAsm);
	}

	switch (retMode)
	{
	case ReturnMode::NONE:
	case ReturnMode::UNREACHABLE: break;
	case ReturnMode::RETURN: {
		if (retValue != nullptr)
		{
			//TODO where... where's the function?

			auto ret = retValue->emitValueCLLR(scopeTable, codeAsm);

			codeAsm.push(cllr::Instruction(cllr::Opcode::RETURN_VALUE, {}, { ret.value }));

		}
		else
		{
			codeAsm.push(cllr::Instruction(cllr::Opcode::RETURN));

		}
		break;
	};
	case ReturnMode::CONTINUE:
		codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { codeAsm.getLoopStart() })); break;
	case ReturnMode::BREAK:
		codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { codeAsm.getLoopEnd() })); break;
	case ReturnMode::PASS:
		//TODO implement
		break;
	case ReturnMode::DISCARD:
		codeAsm.push(cllr::Instruction(cllr::Opcode::DISCARD)); break;
	}

	return 0;
}
