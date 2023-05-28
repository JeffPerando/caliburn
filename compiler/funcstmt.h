
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
		uptr<ParsedType> retPType = nullptr;
		sptr<Type> retType = nullptr;
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
		void resolveSymbols() override
		{
			body->resolveSymbols();

			retPType->resolve(*this);

		}

		ValidationData validate(ref<const std::set<StatementType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			std::set<StatementType> bodyTypes = LOGIC_STMT_TYPES;
			std::set<ReturnMode> bodyModes = { ReturnMode::RETURN };

			bodyTypes.insert(types.begin(), types.end());
			bodyModes.insert(retModes.begin(), retModes.end());

			return body->validate(bodyTypes, bodyModes);
		}

		void getSSAs(ref<cllr::Assembler> codeAsm) override
		{
			funcID = codeAsm.createSSA(cllr::Opcode::FUNCTION);

			body->getSSAs(codeAsm);

		}
		*/
		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			retType->emitDeclCLLR(codeAsm);

			codeAsm.push(funcID, cllr::Opcode::FUNCTION, { (uint32_t)args.size() }, { retType->id });

			body->emitDeclCLLR(codeAsm);

			codeAsm.push(0, cllr::Opcode::FUNCTION_END, {}, { funcID });

		}

	};

}
