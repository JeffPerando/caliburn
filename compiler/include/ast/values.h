
#pragma once

#include "basic.h"
#include "var.h"

#include "cllr/cllr.h"

#include "ast/fn.h"
#include "types/type.h"

namespace caliburn
{
	struct IntLiteralValue : public Value
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

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << lit->str;
		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));
			
			if (auto t = pType.resolve(table))
			{
				//Integer literals parse immediately since there's no loss in precision;
				//Float literals defer parsing since they can lose precision during parsing
				//Why does it matter? I dunno
				uint64_t parsedLit = t->base->parseLiteral(lit->str);

				auto tID = t->emitDeclCLLR(table, codeAsm);

				return codeAsm.pushNew(cllr::Opcode::VALUE_LIT_INT, { (uint32_t)(parsedLit & 0xFFFFFFFF), (uint32_t)((parsedLit >> 32) & 0xFFFFFFFF) }, { tID });
			}

			//TODO complain
			return 0;
		}

	};

	struct FloatLiteralValue : public Value
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

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << lit->str;
		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			//We defer parsing further! Great success!
			auto sID = codeAsm.addString(lit->str.substr(0, lit->str.find_first_of('_')));
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));
			
			if (auto t = pType.resolve(table))
			{
				return codeAsm.pushNew(cllr::Opcode::VALUE_LIT_FP, { sID }, { t->emitDeclCLLR(table, codeAsm) });
			}

			//TODO complain
			return 0;
		}

	};

	struct StringLitValue : public Value
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

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << lit->str;
		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto pType = ParsedType("string");

			if (auto t = pType.resolve(table))
			{
				auto sID = codeAsm.addString(lit->str);

				return codeAsm.pushNew(cllr::Opcode::VALUE_LIT_STR, { sID }, { t->emitDeclCLLR(table, codeAsm) });
			}

			//TODO complain
			return 0;
		}

	};

	struct BoolLitValue : public Value
	{
		const sptr<Token> lit;
		
		BoolLitValue(sptr<Token> v) : Value(ValueType::STR_LITERAL), lit(v)  {}

		sptr<Token> firstTkn() const override
		{
			return lit;
		}

		sptr<Token> lastTkn() const override
		{
			return lit;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << lit->str;
		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto pType = ParsedType("bool");

			if (auto t = pType.resolve(table))
			{
				return codeAsm.pushNew(cllr::Opcode::VALUE_LIT_BOOL, { lit->str == "true" }, { t->emitDeclCLLR(table, codeAsm) });
			}

			//TODO complain
			return 0;
		}

	};

	struct ArrayLitValue : public Value
	{
		sptr<Token> start = nullptr;
		std::vector<sptr<Value>> values;
		sptr<Token> end = nullptr;

		ArrayLitValue() : Value(ValueType::UNKNOWN) {}
		virtual ~ArrayLitValue() {}

		sptr<Token> firstTkn() const override
		{
			return start;
		}

		sptr<Token> lastTkn() const override
		{
			return end;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << '[';

			for (size_t i = 0; i < values.size(); ++i)
			{
				values[i]->prettyPrint(ss);

				if (i + 1 < values.size())
				{
					ss << ", ";
				}

			}

			ss << ']';

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto id = codeAsm.pushNew(cllr::Opcode::VALUE_LIT_ARRAY, { (uint32_t)values.size() }, {});

			for (uint32_t i = 0; i < values.size(); ++i)
			{
				auto const& v = values[i];
				auto vID = v->emitValueCLLR(table, codeAsm);

				codeAsm.pushNew(cllr::Opcode::LIT_ARRAY_ELEM, { i }, { vID });

			}

			return id;
		}

	};

	struct ExpressionValue : public Value
	{
		sptr<Value> lValue = nullptr;
		sptr<Value> rValue = nullptr;
		Operator op = Operator::UNKNOWN;

		ExpressionValue() : Value(ValueType::EXPRESSION) {}

		sptr<Token> firstTkn() const override
		{
			return lValue->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return rValue->lastTkn();
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			const auto cat = OP_CATEGORIES.find(op)->second;

			if (cat == OpCategory::UNARY)
			{
				//TODO complain
				return;
			}

			const auto& opStr = findStrForOp(op);

			lValue->prettyPrint(ss);

			ss << ' ' << opStr << ' ';
				
			rValue->prettyPrint(ss);
			
		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return lValue->isCompileTimeConst() && rValue->isCompileTimeConst();
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto lhs = lValue->emitValueCLLR(table, codeAsm);
			auto rhs = rValue->emitValueCLLR(table, codeAsm);

			auto cllrOp = cllr::Opcode::VALUE_EXPR;

			auto opType = OP_CATEGORIES.at(op);

			if (opType == OpCategory::LOGICAL)
			{
				cllrOp = cllr::Opcode::COMPARE;
			}

			return codeAsm.pushNew(cllrOp, { (uint32_t)op }, { lhs, rhs });
		}

	};

	struct IsAValue : public Value
	{
		sptr<Value> val = nullptr;
		sptr<ParsedType> chkType = nullptr;

		IsAValue() : Value(ValueType::UNKNOWN) {}
		virtual ~IsAValue() {}

		sptr<Token> firstTkn() const override
		{
			return val->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return chkType->lastTkn();
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			val->prettyPrint(ss);

			ss << " is ";

			chkType->prettyPrint(ss);

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return val->isCompileTimeConst();
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			/*
			chkType->emitDeclCLLR(table, codeAsm);
			auto vID = val->emitValueCLLR(table, codeAsm);
			auto tID = chkType->id;

			TODO:
			Emit the bool literal of whether or not type A is an instance of type B.

			*/
			return 0;
			//return codeAsm.pushNew(cllr::Opcode::VALUE_INSTANCEOF, {}, { vID, tID, 0 });;
		}

	};

	struct SubArrayValue : public Value
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

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			array->prettyPrint(ss);

			ss << '[';

			index->prettyPrint(ss);

			ss << ']';

		}

		bool isLValue() const override
		{
			return true;
		}

		bool isCompileTimeConst() const override
		{
			return array->isCompileTimeConst() && index->isCompileTimeConst();
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto aID = array->emitValueCLLR(table, codeAsm);
			auto iID = index->emitValueCLLR(table, codeAsm);

			auto loadID = codeAsm.pushNew(cllr::Opcode::VALUE_SUBARRAY, {}, { aID, iID });

			return loadID;
		}

	};

	struct CastValue : public Value
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

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			lhs->prettyPrint(ss);

			ss << " as ";

			castTarget->prettyPrint(ss);

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return lhs->isCompileTimeConst();
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			if (auto t = castTarget->resolve(table))
			{
				auto vID = lhs->emitValueCLLR(table, codeAsm);
				auto tID = t->emitDeclCLLR(table, codeAsm);

				auto loadID = codeAsm.pushNew(cllr::Opcode::VALUE_CAST, {}, { vID, tID, 0 });

				return loadID;
			}

			//TODO complain
			return 0;
		}

	};

	struct VarReadValue : public Value
	{
		const sptr<Token> varTkn;

		VarReadValue(sptr<Token> v) : Value(ValueType::UNKNOWN), varTkn(v) {}
		virtual ~VarReadValue() {}

		sptr<Token> firstTkn() const override
		{
			return varTkn;
		}

		sptr<Token> lastTkn() const override
		{
			return varTkn;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << varTkn->str;
		}

		//FIXME this got broken by a recent update which removed resolveSymbols()
		bool isLValue() const override
		{
			return true;
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto v = table->find(varTkn->str);

			if (auto varRes = std::get_if<sptr<Variable>>(&v))
			{
				return (*varRes)->emitLoadCLLR(table, codeAsm, 0);
			}
			
			if (auto valRes = std::get_if<sptr<Value>>(&v))
			{
				return (*valRes)->emitValueCLLR(table, codeAsm);
			}

			//TODO complain
			return 0;
		}

	};

	struct MemberReadValue : public Value
	{
		sptr<Value> target = nullptr;
		sptr<Token> memberName = nullptr;

		MemberReadValue() : Value(ValueType::UNKNOWN) {}
		virtual ~MemberReadValue() {}

		sptr<Token> firstTkn() const override
		{
			return target->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return memberName;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			target->prettyPrint(ss);

			ss << '.';

			ss << memberName->str;

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto targetID = target->emitValueCLLR(table, codeAsm);

			if (auto v = std::get_if<sptr<Variable>>(&table->find(memberName->str)))
			{
				return (*v)->emitLoadCLLR(table, codeAsm, targetID);
			}

			//TODO complain
			return 0;
		}

	};
	
	struct UnaryValue : public Value
	{
		Operator op = Operator::UNKNOWN;
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

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << start->str;

			if (op == Operator::SIGN || op == Operator::UNSIGN)
			{
				ss << ' ';
			}

			val->prettyPrint(ss);

			if (end != nullptr)
			{
				ss << end->str;
			}

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return val->isCompileTimeConst();
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_EXPR_UNARY, { (uint32_t)op }, { val->emitValueCLLR(table, codeAsm) });
		}

	};

	struct FnCallValue : public Value
	{
		sptr<Token> name = nullptr;
		sptr<FunctionImpl> fnImpl = nullptr;
		sptr<Value> target = nullptr;
		sptr<GenericArguments> genArgs = nullptr;
		sptr<Token> end = nullptr;

		std::vector<sptr<Value>> args;

		FnCallValue() : Value(ValueType::FUNCTION_CALL) {}
		virtual ~FnCallValue() {}

		sptr<Token> firstTkn() const override
		{
			if (target != nullptr)
				return target->firstTkn();
			return name;
		}

		sptr<Token> lastTkn() const override
		{
			return end;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			if (target != nullptr)
			{
				target->prettyPrint(ss);

				ss << '.';

			}

			ss << name->str;

			genArgs->prettyPrint(ss);

			ss << '(';

			for (size_t i = 0; i < args.size(); ++i)
			{
				const auto& arg = args[i];

				arg->prettyPrint(ss);

				if (i + 1 < args.size())
				{
					ss << ", ";

				}

			}

			ss << ')';

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			if (auto fn = std::get_if<sptr<Function>>(&table->find(name->str)))
			{
				return fnImpl->call(table, codeAsm, args);
			}

			//TODO complain
			return 0;
		}

	};

	struct SetterValue : public Value
	{
		sptr<Value> lhs = nullptr;
		sptr<Value> rhs = nullptr;
		Operator op = Operator::UNKNOWN;

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

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			lhs->prettyPrint(ss);

			ss << ' ';

			if (op != Operator::UNKNOWN)
			{
				ss << findStrForOp(op);

			}

			ss << "= ";

			rhs->prettyPrint(ss);

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			auto lVal = lhs->emitValueCLLR(table, codeAsm);
			auto rVal = rhs->emitValueCLLR(table, codeAsm);

			if (op != Operator::UNKNOWN)
			{
				auto cllrOp = cllr::Opcode::VALUE_EXPR;

				auto opType = OP_CATEGORIES.at(op);

				if (opType == OpCategory::LOGICAL)
				{
					cllrOp = cllr::Opcode::COMPARE;
				}

				rVal = codeAsm.pushNew(cllrOp, { (uint32_t)op }, { lVal, rVal });

			}

			codeAsm.push(0, cllr::Opcode::ASSIGN, {}, { lVal, rVal });

			return 0;
		}

	};

	struct NullValue : public Value
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

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << "null";
		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			//TODO out type
			return codeAsm.pushNew(cllr::Opcode::VALUE_NULL, {}, {});
		}

	};

	struct ZeroValue : public Value
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

		void prettyPrint(ref<std::stringstream> ss) const override {}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return true;
		}

		cllr::SSA emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const override
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_ZERO, {}, {});
		}

	};

}
