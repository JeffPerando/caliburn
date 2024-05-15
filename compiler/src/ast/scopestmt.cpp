
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

void ScopeStmt::declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err)
{
	if (scopeTable != nullptr)
	{
		return;
	}

	scopeTable = new_sptr<SymbolTable>(table);

	for (auto const& stmt : stmts)
	{
		stmt->declareSymbols(scopeTable, err);

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
	cllr::SSA label = codeAsm.getLoopEnd();

	if (label == 0)
	{
		codeAsm.errors->err("Break used outside of a loop", tkn);
	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { label }));
}

void ContinueStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	cllr::SSA label = codeAsm.getLoopStart();

	if (label == 0)
	{
		codeAsm.errors->err("Continue used outside of a loop", tkn);
	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { label }));
}

void DiscardStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	auto h = codeAsm.getSectHeader();

	if (h.op != cllr::Opcode::SHADER_STAGE)
	{
		codeAsm.errors->err("Discard used outside of a shader stage", tkn);
	}

	if (h.operands[0] != (uint32_t)ShaderType::FRAGMENT)
	{
		codeAsm.errors->err("Discard used outside of a fragment shader", tkn);
	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::DISCARD));
}

void ReturnStmt::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	//here for debugging error message formatting
	//codeAsm.errors->err("Single-line error", *this);

	auto h = codeAsm.getSectHeader();

	if (h.op != cllr::Opcode::FUNCTION && h.op != cllr::Opcode::SHADER_STAGE)
	{
		codeAsm.errors->err("Return used outside of a function", first);
	}

	auto retID = h.refs[0];
	auto retType = codeAsm.getType(retID);

	if (retValue != nullptr)
	{
		if (retID == 0)
		{
			auto e = codeAsm.errors->err("Cannot return a value in a void function", *this);

			e->contextStart = h.debugTkn;

			//emit *something*
			codeAsm.push(cllr::Instruction(cllr::Opcode::RETURN));
			return;
		}

		MATCH(retValue->emitValueCLLR(table, codeAsm), cllr::TypedSSA, ret)
		{
			cllr::TypeChecker tc(codeAsm.settings);
			cllr::TypedSSA result;

			if (!tc.check(retType, *ret, result, codeAsm))
			{
				//TODO complain
			}

			codeAsm.push(cllr::Instruction(cllr::Opcode::RETURN_VALUE, {}, { result.value }));

		}
		else
		{
			codeAsm.errors->err("Invalid return value", *retValue);
		}
		
	}
	else
	{
		if (retID != 0)
		{
			codeAsm.errors->err("Must return a value in a non-void function", first);
		}

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
