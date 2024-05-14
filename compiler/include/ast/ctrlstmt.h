
#pragma once

#include "ast/ast.h"
#include "ast/scopestmt.h"
#include "ast/var.h"

namespace caliburn
{
	struct IfStatement : Statement
	{
		Token first;

		sptr<Value> condition = nullptr;
		uptr<ScopeStmt> innerIf = nullptr;
		uptr<ScopeStmt> innerElse = nullptr;
		
		IfStatement() : Statement(StmtType::IF) {}

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			if (innerElse != nullptr)
			{
				return innerElse->lastTkn();
			}

			return innerIf->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override
		{
			innerIf->declareSymbols(table, err);

			if (innerElse != nullptr)
			{
				innerElse->declareSymbols(table, err);
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
		Token first;
		Token index;
		uptr<LocalVariable> indexVar = nullptr;
		sptr<Value> from = nullptr;
		sptr<Value> to = nullptr;
		uptr<ScopeStmt> loop = nullptr;

		ForRangeStatement() : Statement(StmtType::FOR) {}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

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
		Token first;

		sptr<Value> condition = nullptr;
		uptr<ScopeStmt> loop = nullptr;
		bool doWhile = false;

		WhileStatement() : Statement(StmtType::WHILE) {}

		Token firstTkn() const noexcept override
		{
			return first;
		}
		
		Token lastTkn() const noexcept override
		{
			return loop->lastTkn();
		}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

}