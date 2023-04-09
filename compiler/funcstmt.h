
#pragma once

#include "ast.h"
#include "structstmt.h"

namespace caliburn
{
	struct FunctionStatement : public Statement
	{
		Token* first = nullptr;
		Token* name = nullptr;

		cllr::SSA funcID = 0;

		std::vector<Variable*> args;
		ParsedType* retPType = nullptr;
		ConcreteType* retType = nullptr;

		ScopeStatement* body = nullptr;
		
		FunctionStatement() : Statement(StatementType::FUNCTION) {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			return body ? body->last : nullptr;
		}
		/*
		virtual void resolveSymbols() override
		{
			body->resolveSymbols();

			retPType->resolve(*this);

		}

		virtual ValidationData validate(ref<const std::set<StatementType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			std::set<StatementType> bodyTypes = LOGIC_STMT_TYPES;
			std::set<ReturnMode> bodyModes = { ReturnMode::RETURN };

			bodyTypes.insert(types.begin(), types.end());
			bodyModes.insert(retModes.begin(), retModes.end());

			return body->validate(bodyTypes, bodyModes);
		}

		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			funcID = codeAsm.createSSA(cllr::Opcode::FUNCTION);

			body->getSSAs(codeAsm);

		}
		*/
		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			retType->emitDeclCLLR(codeAsm);

			codeAsm.push(funcID, cllr::Opcode::FUNCTION, { (uint32_t)args.size() }, { retType->id });

			body->emitDeclCLLR(codeAsm);

			codeAsm.push(0, cllr::Opcode::FUNCTION_END, {}, { funcID });

		}

	};

}
