
#pragma once

#include "ast.h"
#include "var.h"

namespace caliburn
{
	struct IfStatement : public Statement
	{
		ptr<Token> first = nullptr;

		cllr::SSA id = 0;
		Value* condition = nullptr;
		ScopeStatement* innerIf = nullptr;
		ScopeStatement* innerElse = nullptr;
		cllr::SSA postBranchID = 0;

		IfStatement() : Statement(StatementType::IF) {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			if (innerElse != nullptr)
			{
				return innerElse->lastTkn();
			}

			return innerIf->lastTkn();
		}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override
		{
			innerIf->declareSymbols(table, codeAsm);

			if (innerElse != nullptr)
			{
				innerElse->declareSymbols(table, codeAsm);
			}

		}

		virtual void resolveSymbols(ref<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			condition->resolveSymbols(table);
			innerIf->resolveSymbols(table, codeAsm);

			if (innerElse != nullptr)
			{
				innerElse->resolveSymbols(table, codeAsm);
			}

		}

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override; //see ctrlstmt.cpp
		/*
		virtual ValidationData validate(ref<const std::set<StatementType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			auto innerIfV = innerIf->validate(types, retModes);

			if (innerIfV.status != ValidationStatus::VALID)
			{
				return innerIfV;
			}

			if (innerElse != nullptr)
			{
				return innerElse->validate(types, retModes);
			}

			return ValidationData::valid();
		}

		void eval(std::vector<Statement*>& ast, Module* parent)
		{
			
		}
		*/
	};

	struct ForRangeStatement : public Statement
	{
		ptr<Token> first = nullptr;
		Token* index = nullptr;
		LocalVariable* indexVar = nullptr;
		Value* from = nullptr;
		Value* to = nullptr;
		ScopeStatement* loop = nullptr;

		ForRangeStatement() : Statement(StatementType::FOR) {}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override {}

		virtual void resolveSymbols(ref<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

		/*
		virtual ValidationData validate(ref<const std::set<StatementType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			std::set<StatementType> bodyTypes = LOGIC_STMT_TYPES;
			std::set<ReturnMode> bodyModes = { ReturnMode::CONTINUE, ReturnMode::BREAK };

			bodyTypes.insert(types.begin(), types.end());
			bodyModes.insert(retModes.begin(), retModes.end());

			return loop->validate(bodyTypes, bodyModes);
		}
		*/


	};

	struct WhileStatement : public Statement
	{
		ptr<Token> first = nullptr;

		Value* condition = nullptr;
		ScopeStatement* loop = nullptr;
		bool doWhile = false;

		cllr::SSA start = 0;
		cllr::SSA exit = 0;
		cllr::SSA cont = 0;

		WhileStatement() : Statement(StatementType::WHILE) {}

		virtual Token* firstTkn() const override
		{
			return first;
		}
		
		virtual Token* lastTkn() const override
		{
			return loop->lastTkn();
		}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override {}

		virtual void resolveSymbols(ref<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			condition->resolveSymbols(table);
			loop->resolveSymbols(table, codeAsm);

		}
		
		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

	};

}