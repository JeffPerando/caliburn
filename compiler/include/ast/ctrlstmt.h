
#pragma once

#include "ast.h"
#include "scopestmt.h"
#include "var.h"

namespace caliburn
{
	struct IfStatement : Statement
	{
		sptr<Token> first = nullptr;

		sptr<Value> condition = nullptr;
		uptr<ScopeStmt> innerIf = nullptr;
		uptr<ScopeStmt> innerElse = nullptr;
		
		IfStatement() : Statement(StmtType::IF) {}

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

		void declareSymbols(sptr<SymbolTable> table) override
		{
			innerIf->declareSymbols(table);

			if (innerElse != nullptr)
			{
				innerElse->declareSymbols(table);
			}

		}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		/*
		ValidationData validate(ref<const std::set<StmtType>> types, ref<const std::set<ReturnMode>> retModes) const override
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

	struct ForRangeStatement : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> index = nullptr;
		uptr<LocalVariable> indexVar = nullptr;
		sptr<Value> from = nullptr;
		sptr<Value> to = nullptr;
		uptr<ScopeStmt> loop = nullptr;

		ForRangeStatement() : Statement(StmtType::FOR) {}

		void declareSymbols(sptr<SymbolTable> table) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		/*
		ValidationData validate(ref<const std::set<StmtType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			std::set<StmtType> bodyTypes = LOGIC_STMT_TYPES;
			std::set<ReturnMode> bodyModes = { ReturnMode::CONTINUE, ReturnMode::BREAK };

			bodyTypes.insert(types.begin(), types.end());
			bodyModes.insert(retModes.begin(), retModes.end());

			return loop->validate(bodyTypes, bodyModes);
		}
		*/

	};

	struct WhileStatement : Statement
	{
		sptr<Token> first = nullptr;

		sptr<Value> condition = nullptr;
		uptr<ScopeStmt> loop = nullptr;
		bool doWhile = false;

		WhileStatement() : Statement(StmtType::WHILE) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}
		
		sptr<Token> lastTkn() const override
		{
			return loop->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

}