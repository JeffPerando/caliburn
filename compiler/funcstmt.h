
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
		
		FunctionStatement(Statement* parent) : Statement(StatementType::FUNCTION, parent) {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			return body ? body->last : nullptr;
		}

		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			funcID = codeAsm.createSSA(cllr::Opcode::FUNCTION);

			body->getSSAs(codeAsm);

		}

		virtual void declSymbols(SymbolTable& table) override
		{
			table.add(name->str, SymbolType::FUNCTION, this);

			body->declSymbols(table);

		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			body->resolveSymbols(table);

			retPType->resolve(table);

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			retType->emitDeclCLLR(codeAsm);

			codeAsm.push(funcID, cllr::Opcode::FUNCTION, { (uint32_t)args.size(), retType->id, 0 });

			body->emitDeclCLLR(codeAsm);

			codeAsm.push(0, cllr::Opcode::FUNCTION_END, {funcID, 0, 0});

		}

	};

	struct MemberFunctionStatement : public FunctionStatement
	{
	private:
		SymbolTable* memberTable = nullptr;

	public:
		MemberFunctionStatement(Statement* parent) : FunctionStatement(parent) {}

		virtual void declSymbols(SymbolTable& table) override
		{
			memberTable = table.makeChild("members");

			auto& members = ((StructStatement*)parent)->members;

			for (auto mem : members)
			{
				memberTable->add(mem->name->str, SymbolType::VALUE, mem);

			}

			FunctionStatement::declSymbols(*memberTable);

		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			FunctionStatement::resolveSymbols(*memberTable);

		}

	};

}
