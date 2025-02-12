
#pragma once

#include "ast/ast.h"

namespace caliburn
{
	struct SetStmt : Expr
	{
		sptr<Expr> lhs;
		sptr<Expr> rhs;

		SetStmt() : Expr(ExprType::SETTER) {}
		SetStmt(sptr<Expr> lval, sptr<Expr> rval) :
			Expr(ExprType::SETTER), lhs(lval), rhs(rval) {}

		virtual ~SetStmt() = default;

		Token firstTkn() const noexcept override
		{
			return lhs->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			return rhs->lastTkn();
		}

		void prettyPrint(out<std::stringstream> ss) const override
		{
			//FIXME
		}

		bool isLValue() const
		{
			return false;
		}

		bool isCompileTimeConst() const
		{
			return false;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const;

	};

}