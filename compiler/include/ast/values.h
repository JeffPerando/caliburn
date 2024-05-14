
#pragma once

#include "basic.h"

#include "ast/ast.h"
#include "ast/type.h"

#include "cllr/cllr.h"

namespace caliburn
{
	struct IntLiteralValue : Value
	{
		const Token lit;

		IntLiteralValue(in<Token> l) : Value(ValueType::INT_LITERAL), lit(l) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct FloatLiteralValue : Value
	{
		const Token lit;

		FloatLiteralValue(in<Token> l) : Value(ValueType::FLOAT_LITERAL), lit(l) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct StringLitValue : Value
	{
		const Token lit;

		StringLitValue(in<Token> str) : Value(ValueType::STR_LITERAL), lit(str) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct BoolLitValue : Value
	{
		const Token lit;
		
		BoolLitValue(in<Token> v) : Value(ValueType::BOOL_LITERAL), lit(v)  {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct ArrayLitValue : Value
	{
		Token start;
		std::vector<sptr<Value>> values;
		Token end;

		ArrayLitValue() : Value(ValueType::ARRAY_LITERAL) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct ExpressionValue : Value
	{
		sptr<Value> lValue = nullptr;
		sptr<Value> rValue = nullptr;
		Operator op = Operator::NONE;

		ExpressionValue() : Value(ValueType::EXPRESSION) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct SubArrayValue : Value
	{
		const sptr<Value> array;
		const sptr<Value> index;
		const Token last;

		SubArrayValue(sptr<Value> a, sptr<Value> i, in<Token> l) :
			Value(ValueType::SUB_ARRAY), array(a), index(i), last(l) {}

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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct CastValue : Value
	{
		sptr<Value> lhs = nullptr;
		sptr<ParsedType> castTarget = nullptr;

		CastValue() : Value(ValueType::CAST) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct VarReadValue : Value
	{
		const Token varTkn;
		const std::string varStr;

		VarReadValue(in<Token> v) : Value(ValueType::VAR_READ), varTkn(v), varStr(v.str) {}
		VarReadValue(in<std::string> v) : Value(ValueType::VAR_READ), varStr(v) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct MemberReadDirectValue : Value
	{
		const sptr<Value> target;
		//take a string instead of a token so we can use members directly within methods
		const std::string mem;

		MemberReadDirectValue(sptr<Value> t, in<std::string> m) : Value(ValueType::MEMBER_READ), target(t), mem(m) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};
	
	struct MemberReadChainValue : Value
	{
		const sptr<Value> target;
		
		std::vector<Token> mems;

		MemberReadChainValue(sptr<Value> t) : Value(ValueType::MEMBER_READ), target(t) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct UnaryValue : Value
	{
		Operator op = Operator::NONE;
		Token start;
		sptr<Value> val = nullptr;
		Token end;

		UnaryValue() : Value(ValueType::UNKNOWN) {}
		UnaryValue(in<Token> s, Operator o, sptr<Value> v) : start(s), op(o), val(v), Value(ValueType::UNKNOWN) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct FnCallValue : Value
	{
		const sptr<Value> name;

		sptr<GenericArguments> genArgs;
		std::vector<sptr<Value>> args;
		Token end;

		FnCallValue(sptr<Value> n) : Value(ValueType::FUNCTION_CALL), name(n) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct MethodCallValue : Value
	{
		const sptr<Value> target;
		const Token name;

		sptr<GenericArguments> genArgs;
		std::vector<sptr<Value>> args;
		Token end;

		MethodCallValue(sptr<Value> t, in<Token> n) : Value(ValueType::FUNCTION_CALL), target(t), name(n) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct SetterValue : Value
	{
		sptr<Value> lhs = nullptr;
		sptr<Value> rhs = nullptr;
		Operator op = Operator::NONE;

		SetterValue() : Value(ValueType::UNKNOWN) {}
		virtual ~SetterValue() = default;

		Token firstTkn() const noexcept override
		{
			return lhs->firstTkn();
		}

		Token lastTkn() const noexcept override
		{
			return rhs->firstTkn();
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct NullValue : Value
	{
		const Token lit;//I don't even know what to do with this

		NullValue(in<Token> v) : Value(ValueType::UNKNOWN), lit(v) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct ZeroValue : Value
	{
		ZeroValue() : Value(ValueType::UNKNOWN) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct SignValue : Value
	{
		const Token first;
		const sptr<Value> target;

		SignValue(in<Token> f, sptr<Value> v) : Value(ValueType::UNKNOWN), first(f), target(v) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

	struct UnsignValue : Value
	{
		const Token first;
		const sptr<Value> target;

		UnsignValue(in<Token> f, sptr<Value> v) : Value(ValueType::UNKNOWN), first(f), target(v) {}
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

		ValueResult emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const override;

	};

}
