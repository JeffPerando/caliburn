
#pragma once

#include "ast/ast.h"
#include "ast/scopestmt.h"
#include "ast/var.h"

namespace caliburn
{
	struct IfStatement : Expr
	{
		Token first;

		sptr<Expr> condition = nullptr;
		sptr<Expr> innerIf = nullptr;
		sptr<Expr> innerElse = nullptr;
		
		IfStatement() : Expr(ExprType::IF) {}

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

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

		/*
		ValidationData validate(ref<const std::set<ExprType>> types, ref<const std::set<ReturnMode>> retModes) const override
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

		void eval(std::vector<Expr*>& ast, Module* parent)
		{
			
		}
		*/

	};

	struct ForRangeStatement : Expr
	{
		Token first;
		Token index;
		uptr<LocalVariable> indexVar = nullptr;
		sptr<Expr> from = nullptr;
		sptr<Expr> to = nullptr;
		uptr<ScopeStmt> loop = nullptr;

		ForRangeStatement() : Expr(ExprType::FOR) {}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

		/*
		ValidationData validate(ref<const std::set<ExprType>> types, ref<const std::set<ReturnMode>> retModes) const override
		{
			std::set<ExprType> bodyTypes = LOGIC_STMT_TYPES;
			std::set<ReturnMode> bodyModes = { ReturnMode::CONTINUE, ReturnMode::BREAK };

			bodyTypes.insert(types.begin(), types.end());
			bodyModes.insert(retModes.begin(), retModes.end());

			return loop->validate(bodyTypes, bodyModes);
		}
		*/

	};

	struct WhileStatement : Expr
	{
		Token first;

		sptr<Expr> condition = nullptr;
		uptr<ScopeStmt> loop = nullptr;
		bool doWhile = false;

		WhileStatement() : Expr(ExprType::WHILE) {}

		Token firstTkn() const noexcept override
		{
			return first;
		}
		
		Token lastTkn() const noexcept override
		{
			return loop->lastTkn();
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

}