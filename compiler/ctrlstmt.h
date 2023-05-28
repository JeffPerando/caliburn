
#pragma once

#include "ast.h"
#include "var.h"

namespace caliburn
{
	struct IfStatement : public Statement
	{
		sptr<Token> first = nullptr;

		uptr<Value> condition = nullptr;
		uptr<ScopeStatement> innerIf = nullptr;
		uptr<ScopeStatement> innerElse = nullptr;
		
		IfStatement() : Statement(StatementType::IF) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			if (innerElse != nullptr)
			{
				return innerElse->lastTkn();
			}

			return innerIf->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			innerIf->declareSymbols(table, codeAsm);

			if (innerElse != nullptr)
			{
				innerElse->declareSymbols(table, codeAsm);
			}

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			condition->resolveSymbols(table);
			innerIf->resolveSymbols(table, codeAsm);

			if (innerElse != nullptr)
			{
				innerElse->resolveSymbols(table, codeAsm);
			}

		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override; //see ctrlstmt.cpp

		/*
		ValidationData validate(ref<const std::set<StatementType>> types, ref<const std::set<ReturnMode>> retModes) const override
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
		sptr<Token> first = nullptr;
		sptr<Token> index = nullptr;
		uptr<LocalVariable> indexVar = nullptr;
		uptr<Value> from = nullptr;
		uptr<Value> to = nullptr;
		uptr<ScopeStatement> loop = nullptr;

		ForRangeStatement() : Statement(StatementType::FOR) {}

		void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{

		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override;

		/*
		ValidationData validate(ref<const std::set<StatementType>> types, ref<const std::set<ReturnMode>> retModes) const override
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
		sptr<Token> first = nullptr;

		uptr<Value> condition = nullptr;
		uptr<ScopeStatement> loop = nullptr;
		bool doWhile = false;

		WhileStatement() : Statement(StatementType::WHILE) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}
		
		sptr<Token> lastTkn() const override
		{
			return loop->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			condition->resolveSymbols(table);
			loop->resolveSymbols(table, codeAsm);

		}
		
		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override;

	};

}