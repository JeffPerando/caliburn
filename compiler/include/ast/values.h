
#pragma once

#include "basic.h"

#include "ast/ast.h"
#include "ast/type.h"

#include "cllr/cllr.h"

namespace caliburn
{
	struct IntLiteralValue : Expr
	{
		const Token lit;

		IntLiteralValue(in<Token> l) : Expr(ExprType::INT_LITERAL), lit(l) {}
		virtual ~IntLiteralValue() = default;

		Token firstTkn() const noexcept override
		{
			return lit;
		}

		Token lastTkn() const noexcept override
		{
			return lit;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct FloatLiteralValue : Expr
	{
		const Token lit;

		FloatLiteralValue(in<Token> l) : Expr(ExprType::FLOAT_LITERAL), lit(l) {}
		virtual ~FloatLiteralValue() = default;

		Token firstTkn() const noexcept override
		{
			return lit;
		}

		Token lastTkn() const noexcept override
		{
			return lit;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct StringLitValue : Expr
	{
		const Token lit;

		StringLitValue(in<Token> str) : Expr(ExprType::STR_LITERAL), lit(str) {}
		virtual ~StringLitValue() = default;

		Token firstTkn() const noexcept override
		{
			return lit;
		}

		Token lastTkn() const noexcept override
		{
			return lit;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct BoolLitValue : Expr
	{
		const Token lit;
		
		BoolLitValue(in<Token> v) : Expr(ExprType::BOOL_LITERAL), lit(v)  {}
		virtual ~BoolLitValue() = default;

		Token firstTkn() const noexcept override
		{
			return lit;
		}

		Token lastTkn() const noexcept override
		{
			return lit;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct ArrayLitValue : Expr
	{
		Token start;
		std::vector<sptr<Expr>> values;
		Token end;

		ArrayLitValue() : Expr(ExprType::ARRAY_LITERAL) {}
		virtual ~ArrayLitValue() = default;

		Token firstTkn() const noexcept override
		{
			return start;
		}

		Token lastTkn() const noexcept override
		{
			return end;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			/* TODO consider
			for (auto& v : values)
			{
				if (!v->isCompileTimeConst())
				{
					return false;
				}

			}
			*/
			return true;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct ExpressionValue : Expr
	{
		sptr<Expr> lValue = nullptr;
		sptr<Expr> rValue = nullptr;
		Operator op = Operator::NONE;

		ExpressionValue() : Expr(ExprType::EXPRESSION) {}
		ExpressionValue(sptr<Expr> lhs, Operator o, sptr<Expr> rhs) :
			Expr(ExprType::EXPRESSION), lValue(lhs), op(o), rValue(rhs) {}
		virtual ~ExpressionValue() = default;

		Token firstTkn() const noexcept override
		{
			return lValue->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			return rValue->lastTkn();
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return lValue->isCompileTimeConst() && rValue->isCompileTimeConst();
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct SubArrayValue : Expr
	{
		const sptr<Expr> array;
		const sptr<Expr> index;
		const Token last;

		SubArrayValue(sptr<Expr> a, sptr<Expr> i, in<Token> l) :
			Expr(ExprType::SUB_ARRAY), array(a), index(i), last(l) {}

		virtual ~SubArrayValue() = default;

		Token firstTkn() const noexcept override
		{
			return array->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			return last;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return true;
		}

		bool isCompileTimeConst() const override
		{
			return array->isCompileTimeConst() && index->isCompileTimeConst();
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct CastValue : Expr
	{
		sptr<Expr> lhs = nullptr;
		sptr<ParsedType> castTarget = nullptr;

		CastValue() : Expr(ExprType::CAST) {}
		virtual ~CastValue() = default;

		Token firstTkn() const noexcept override
		{
			return lhs->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			return castTarget->lastTkn();
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return lhs->isCompileTimeConst();
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct VarReadValue : Expr
	{
		const Token varTkn;
		const std::string varStr;

		VarReadValue(in<Token> v) : Expr(ExprType::VAR_READ), varTkn(v), varStr(v.str) {}
		VarReadValue(in<std::string> v) : Expr(ExprType::VAR_READ), varStr(v) {}
		virtual ~VarReadValue() = default;

		Token firstTkn() const noexcept override
		{
			return varTkn;
		}

		Token lastTkn() const noexcept override
		{
			return varTkn;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return true;
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct MemberReadDirectValue : Expr
	{
		const sptr<Expr> target;
		//take a string instead of a token so we can use members directly within methods
		const std::string mem;

		MemberReadDirectValue(sptr<Expr> t, in<std::string> m) : Expr(ExprType::MEMBER_READ), target(t), mem(m) {}
		virtual ~MemberReadDirectValue() = default;

		Token firstTkn() const noexcept override
		{
			return target->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			//FIXME this is technically incorrect
			return target->lastTkn();
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return true;
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};
	
	struct MemberReadChainValue : Expr
	{
		const sptr<Expr> target;
		
		std::vector<Token> mems;

		MemberReadChainValue(sptr<Expr> t) : Expr(ExprType::MEMBER_READ), target(t) {}
		virtual ~MemberReadChainValue() = default;

		Token firstTkn() const noexcept override
		{
			return target->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			return mems.back();
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return true;
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct UnaryValue : Expr
	{
		Operator op = Operator::NONE;
		Token start;
		sptr<Expr> val = nullptr;
		Token end;

		UnaryValue() : Expr(ExprType::UNKNOWN) {}
		UnaryValue(in<Token> s, Operator o, sptr<Expr> v) : start(s), op(o), val(v), Expr(ExprType::UNKNOWN) {}
		virtual ~UnaryValue() = default;

		Token firstTkn() const noexcept override
		{
			return start;
		}

		Token lastTkn() const noexcept override
		{
			if (end.type == TokenType::UNKNOWN)
				return val->lastTkn();
			return end;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return val->isCompileTimeConst();
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct FnCallValue : Expr
	{
		const sptr<Expr> name;

		sptr<GenericArguments> genArgs;
		std::vector<sptr<Expr>> args;
		Token end;

		FnCallValue(sptr<Expr> n) : Expr(ExprType::FUNCTION_CALL), name(n) {}
		virtual ~FnCallValue() = default;

		Token firstTkn() const noexcept override
		{
			return name->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			return end;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct MethodCallValue : Expr
	{
		const sptr<Expr> target;
		const Token name;

		sptr<GenericArguments> genArgs;
		std::vector<sptr<Expr>> args;
		Token end;

		MethodCallValue(sptr<Expr> t, in<Token> n) : Expr(ExprType::FUNCTION_CALL), target(t), name(n) {}
		virtual ~MethodCallValue() = default;

		Token firstTkn() const noexcept override
		{
			return target->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			return end;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct NullValue : Expr
	{
		const Token lit;//I don't even know what to do with this

		NullValue(in<Token> v) : Expr(ExprType::UNKNOWN), lit(v) {}
		virtual ~NullValue() = default;

		Token firstTkn() const noexcept override
		{
			return lit;
		}

		Token lastTkn() const noexcept override
		{
			return lit;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct ZeroValue : Expr
	{
		ZeroValue() : Expr(ExprType::UNKNOWN) {}
		virtual ~ZeroValue() = default;

		Token firstTkn() const noexcept override
		{
			return Token();
		}

		Token lastTkn() const noexcept override
		{
			return Token();
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct SignValue : Expr
	{
		const Token first;
		const sptr<Expr> target;

		SignValue(in<Token> f, sptr<Expr> v) : Expr(ExprType::UNKNOWN), first(f), target(v) {}
		virtual ~SignValue() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return target->lastTkn();
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return target->isCompileTimeConst();
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct UnsignValue : Expr
	{
		const Token first;
		const sptr<Expr> target;

		UnsignValue(in<Token> f, sptr<Expr> v) : Expr(ExprType::UNKNOWN), first(f), target(v) {}
		virtual ~UnsignValue() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return target->lastTkn();
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return target->isCompileTimeConst();
		}

		ValueResult emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

}
