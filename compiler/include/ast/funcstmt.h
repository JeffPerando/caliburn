
#pragma once

#include "ast.h"
#include "structstmt.h"

namespace caliburn
{
	struct FunctionStatement : public Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;

		cllr::SSA funcID = 0;

		std::vector<uptr<Variable>> args;
		sptr<ParsedType> retType = nullptr;
		uptr<ScopeStatement> body = nullptr;
		
		FunctionStatement() : Statement(StatementType::FUNCTION) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return body ? body->last : nullptr;
		}
		/*
		ValidationData validate(ref<const std::set<StatementType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			std::set<StatementType> bodyTypes = LOGIC_STMT_TYPES;
			std::set<ReturnMode> bodyModes = { ReturnMode::RETURN };

			bodyTypes.insert(types.begin(), types.end());
			bodyModes.insert(retModes.begin(), retModes.end());

			return body->validate(bodyTypes, bodyModes);
		}
		*/
		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			auto tID = retType->resolve(table)->emitDeclCLLR(table, codeAsm);

			codeAsm.push(funcID, cllr::Opcode::FUNCTION, { (uint32_t)args.size() }, { tID });

			body->emitDeclCLLR(table, codeAsm);

			codeAsm.push(0, cllr::Opcode::FUNCTION_END, {}, { funcID });

		}

	};

}
