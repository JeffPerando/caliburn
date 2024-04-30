

#pragma once

#include "basic.h"
#include "var.h"

#include "cllr/cllr.h"

#include "types/type.h"

namespace caliburn
{
	struct IntLiteralValue : Value
	{
		const sptr<Token> lit;

		IntLiteralValue(sptr<Token> l) : Value(ValueType::INT_LITERAL), lit(l) {}

		sptr<Token> firstTkn() const override
		{
			return lit;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> lit;

		FloatLiteralValue(sptr<Token> l) : Value(ValueType::FLOAT_LITERAL), lit(l) {}

		sptr<Token> firstTkn() const override
		{
			return lit;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> lit;

		StringLitValue(sptr<Token> str) : Value(ValueType::STR_LITERAL), lit(str) {}

		sptr<Token> firstTkn() const override
		{
			return lit;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> lit;
		
		BoolLitValue(sptr<Token> v) : Value(ValueType::BOOL_LITERAL), lit(v)  {}

		sptr<Token> firstTkn() const override
		{
			return lit;
		}

		sptr<Token> lastTkn() const override
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
		sptr<Token> start = nullptr;
		std::vector<sptr<Value>> values;
		sptr<Token> end = nullptr;

		ArrayLitValue() : Value(ValueType::ARRAY_LITERAL) {}
		virtual ~ArrayLitValue() {}

		sptr<Token> firstTkn() const override
		{
			return start;
		}

		sptr<Token> lastTkn() const override
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

		sptr<Token> firstTkn() const override
		{
			return lValue->firstTkn();
		}

		sptr<Token> lastTkn() const override
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
		sptr<Value> array = nullptr;
		sptr<Value> index = nullptr;
		sptr<Token> last = nullptr;

		SubArrayValue() : Value(ValueType::SUB_ARRAY) {}

		sptr<Token> firstTkn() const override
		{
			return array->firstTkn();
		}

		sptr<Token> lastTkn() const override
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
		virtual ~CastValue() {}

		sptr<Token> firstTkn() const override
		{
			return lhs->firstTkn();
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> varTkn;

		VarReadValue(sptr<Token> v) : Value(ValueType::VAR_READ), varTkn(v) {}
		virtual ~VarReadValue() {}

		sptr<Token> firstTkn() const override
		{
			return varTkn;
		}

		sptr<Token> lastTkn() const override
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

	struct VarChainValue : Value
	{
		sptr<Value> target = nullptr;
		std::vector<sptr<Token>> chain;

		VarChainValue() : Value(ValueType::UNKNOWN) {}
		virtual ~VarChainValue() {}

		sptr<Token> firstTkn() const override
		{
			if (target != nullptr)
			{
				return target->firstTkn();
			}

			return chain.front();
		}

		sptr<Token> lastTkn() const override
		{
			return chain.back();
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
		sptr<Token> start = nullptr;
		sptr<Value> val = nullptr;
		sptr<Token> end = nullptr;

		UnaryValue() : Value(ValueType::UNKNOWN) {}
		UnaryValue(sptr<Token> s, Operator o, sptr<Value> v) : start(s), op(o), val(v), Value(ValueType::UNKNOWN) {}
		virtual ~UnaryValue() {}

		sptr<Token> firstTkn() const override
		{
			return start;
		}

		sptr<Token> lastTkn() const override
		{
			if (end == nullptr)
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
		sptr<Value> name = nullptr;
		sptr<Value> target = nullptr;
		sptr<GenericArguments> genArgs = nullptr;
		sptr<Token> end = nullptr;

		std::vector<sptr<Value>> args;

		FnCallValue() : Value(ValueType::FUNCTION_CALL) {}
		virtual ~FnCallValue() {}

		sptr<Token> firstTkn() const override
		{
			if (target != nullptr)
			{
				return target->firstTkn();
			}

			return name->firstTkn();
		}

		sptr<Token> lastTkn() const override
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
		virtual ~SetterValue() {}

		sptr<Token> firstTkn() const override
		{
			return lhs->firstTkn();
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> lit;//I don't even know what to do with this

		NullValue(sptr<Token> v) : Value(ValueType::UNKNOWN), lit(v) {}
		virtual ~NullValue() {}

		sptr<Token> firstTkn() const override
		{
			return lit;
		}

		sptr<Token> lastTkn() const override
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
		virtual ~ZeroValue() {}

		sptr<Token> firstTkn() const override
		{
			return nullptr;
		}

		sptr<Token> lastTkn() const override
		{
			return nullptr;
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
		const sptr<Token> first;
		const sptr<Value> target;

		SignValue(sptr<Token> f, sptr<Value> v) : Value(ValueType::UNKNOWN), first(f), target(v) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
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
		const sptr<Token> first;
		const sptr<Value> target;

		UnsignValue(sptr<Token> f, sptr<Value> v) : Value(ValueType::UNKNOWN), first(f), target(v) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
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
