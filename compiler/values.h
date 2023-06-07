
#pragma once

#include "basic.h"
#include "cllr.h"
#include "type.h"

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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

			if (!pType.resolve(table))
			{
				//TODO complain
				return;
			}

			type = pType.real();

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			type->emitDeclCLLR(codeAsm);

			//Integer literals parse immediately since there's no loss in precision;
			//Float literals defer parsing since they can lose precision during parsing
			//Why does it matter? I dunno
			auto parsedLit = type->parseLiteral(lit->str);

			return codeAsm.pushNew(cllr::Opcode::VALUE_INT_LIT, { (uint32_t)(parsedLit & 0xFFFFFFFF), (uint32_t)((parsedLit >> 32) & 0xFFFFFFFF) }, { type->id });
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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));
			if (!pType.resolve(table))
			{
				//TODO complain
				return;
			}
			type = pType.real();

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			type->emitDeclCLLR(codeAsm);

			//We defer parsing further! Great success!
			auto sID = codeAsm.addString(lit->str.substr(0, lit->str.find_first_of('_')));

			return codeAsm.pushNew(cllr::Opcode::VALUE_FP_LIT, { sID }, { type->id });
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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			auto t = table->find("string");
			auto tResult = std::get_if<sptr<Type>>(&t);

			if (tResult == nullptr)
			{
				//TODO complain
				return;
			}

			type = *tResult;

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			type->emitDeclCLLR(codeAsm);

			auto sID = codeAsm.addString(lit->str);
			
			return codeAsm.pushNew(cllr::Opcode::VALUE_STR_LIT, { sID }, { type->id });
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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			auto t = table->find("bool");
			auto tResult = std::get_if<sptr<Type>>(&t);

			if (tResult == nullptr)
			{
				//TODO complain
				return;
			}

			type = *tResult;

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			type->emitDeclCLLR(codeAsm);

			return codeAsm.pushNew(cllr::Opcode::VALUE_BOOL_LIT, { lit->str == "true" }, { type->id });
		}

	};

	struct ArrayLitValue : public Value
	{
		sptr<Token> start = nullptr;
		std::vector<uptr<Value>> values;
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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			for (auto const& v : values)
			{
				if (v != nullptr)
				{
					v->resolveSymbols(table);

				}

			}

			//TODO type inference
		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto id = codeAsm.pushNew(cllr::Opcode::VALUE_ARRAY_LIT, { (uint32_t)values.size() }, {});

			for (auto const& v : values)
			{
				if (v != nullptr)
				{
					//TODO figure out VALUE_ARRAY_LIT semantics
					v->emitValueCLLR(codeAsm);

				}

			}

			return id;
		}

	};

	struct ExpressionValue : public Value
	{
		uptr<Value> lValue = nullptr;
		uptr<Value> rValue = nullptr;
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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			lValue->resolveSymbols(table);
			rValue->resolveSymbols(table);
			
			auto compat = lValue->type->isCompatible(op, rValue->type);

			//TODO check compatibility

			type = lValue->type;

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto lhs = lValue->emitValueCLLR(codeAsm);
			auto rhs = rValue->emitValueCLLR(codeAsm);

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
		uptr<Value> val = nullptr;
		uptr<ParsedType> chkPType = nullptr;
		sptr<Type> chkType = nullptr;

		IsAValue() : Value(ValueType::CAST) {}
		virtual ~IsAValue() {}

		sptr<Token> firstTkn() const override
		{
			return val->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return chkPType->lastTkn();
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			val->prettyPrint(ss);

			ss << " is ";

			chkPType->prettyPrint(ss);

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return val->isCompileTimeConst();
		}

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			val->resolveSymbols(table);
			chkPType->resolve(table);
			chkType = chkPType->real();

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			chkType->emitDeclCLLR(codeAsm);
			auto vID = val->emitValueCLLR(codeAsm);
			auto tID = chkType->id;

			return codeAsm.pushNew(cllr::Opcode::VALUE_INSTANCEOF, {}, { vID, tID, 0 });;
		}

	};

	struct SubArrayValue : public Value
	{
		uptr<Value> array = nullptr;
		uptr<Value> index = nullptr;
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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			array->resolveSymbols(table);
			index->resolveSymbols(table);

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto aID = array->emitValueCLLR(codeAsm);
			auto iID = index->emitValueCLLR(codeAsm);

			auto loadID = codeAsm.pushNew(cllr::Opcode::VALUE_SUBARRAY, {}, { aID, iID });

			return loadID;
		}

	};

	struct CastValue : public Value
	{
		uptr<Value> lhs = nullptr;
		uptr<ParsedType> resultPType = nullptr;

		CastValue() : Value(ValueType::CAST) {}
		virtual ~CastValue() {}

		sptr<Token> firstTkn() const override
		{
			return lhs->firstTkn();
		}

		sptr<Token> lastTkn() const override
		{
			return resultPType->lastTkn();
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			lhs->prettyPrint(ss);

			ss << " as ";

			resultPType->prettyPrint(ss);

		}

		bool isLValue() const override
		{
			return false;
		}

		bool isCompileTimeConst() const override
		{
			return lhs->isCompileTimeConst();
		}

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			resultPType->resolve(table);
			type = resultPType->real();
			lhs->resolveSymbols(table);
			
		}
		
		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto vID = lhs->emitValueCLLR(codeAsm);

			auto loadID = codeAsm.pushNew(cllr::Opcode::VALUE_CAST, {}, { vID, type->id, 0 });

			return loadID;
		}

	};

	struct VarReadValue : public Value
	{
		const sptr<Token> var;

		Symbol varSym = nullptr;

		VarReadValue(sptr<Token> v) : Value(ValueType::UNKNOWN), var(v) {}
		virtual ~VarReadValue() {}

		sptr<Token> firstTkn() const override
		{
			return var;
		}

		sptr<Token> lastTkn() const override
		{
			return var;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << var->str;
		}

		bool isLValue() const override
		{
			return std::holds_alternative<sptr<Variable>>(varSym);
		}

		bool isCompileTimeConst() const override
		{
			return false;
		}

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			varSym = table->find(var->str);

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto varRes = std::get_if<sptr<Variable>>(&varSym);

			if (varRes != nullptr)
			{
				return (*varRes)->emitLoadCLLR(codeAsm, 0);
			}
			
			auto valRes = std::get_if<sptr<Value>>(&varSym);

			if (valRes != nullptr)
			{
				return (*valRes)->emitValueCLLR(codeAsm);
			}

			//TODO complain
			return 0;
		}

	};

	struct MemberReadValue : public Value
	{
		uptr<Value> target = nullptr;
		sptr<Token> memberName = nullptr;
		Symbol member = nullptr;

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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			target->resolveSymbols(table);

			member = target->type->memberTable->find(memberName->str);

			if (!std::holds_alternative<sptr<Variable>>(member))
			{
				//TODO complain
			}

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto vID = target->emitValueCLLR(codeAsm);

			return std::get<sptr<Variable>>(member)->emitLoadCLLR(codeAsm, vID);
		}

	};
	
	struct UnaryValue : public Value
	{
		Operator op = Operator::UNKNOWN;
		sptr<Token> start = nullptr;
		uptr<Value> val = nullptr;
		sptr<Token> end = nullptr;

		UnaryValue() : Value(ValueType::UNKNOWN) {}
		UnaryValue(sptr<Token> s, Operator o, uptr<Value> v) : start(s), op(o), val(std::move(v)), Value(ValueType::UNKNOWN) {}
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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			val->resolveSymbols(table);
		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto vID = val->emitValueCLLR(codeAsm);
;			
			return codeAsm.pushNew(cllr::Opcode::VALUE_EXPR_UNARY, { (uint32_t)op }, { vID });
		}

	};

	struct FnCallValue : public Value
	{
		sptr<Token> name = nullptr;
		cllr::SSA funcID = 0;
		uptr<Value> target = nullptr;
		GenericArguments genArgs;
		std::vector<uptr<Value>> args;
		sptr<Token> end = nullptr;

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

			genArgs.prettyPrint(ss);

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
			return true;
		}

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			ptr<const SymbolTable> lookup = table.get();
			/*
			for (auto const& p : pGenerics)
			{
				generics.push_back(p->resolve(table));
			}
			*/
			if (target != nullptr)
			{
				lookup = target->type->memberTable.get();

				args.push_back(std::move(target));

				target = nullptr;

			}

			for (auto const& a : args)
			{
				a->resolveSymbols(table);
			}

			auto fn = lookup->find(name->str);

			if (std::holds_alternative<sptr<Function>>(fn))
			{
				funcID = 0;//lookup + create generic variant
			}
			else
			{
				//TODO complain
				return;
			}

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto callID = codeAsm.pushNew(cllr::Opcode::CALL, {(uint32_t)args.size() + (target != nullptr)}, { funcID });

			for (uint32_t i = 0; i < args.size(); ++i)
			{
				auto const& arg = args.at(i);
				auto aID = arg->emitValueCLLR(codeAsm);

				codeAsm.pushNew(cllr::Opcode::CALL_ARG, { i }, { aID, callID });

			}

			return callID;
		}

	};

	struct SetterValue : public Value
	{
		uptr<Value> lhs = nullptr;
		uptr<Value> rhs = nullptr;
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

		void resolveSymbols(sptr<const SymbolTable> table) override
		{
			lhs->resolveSymbols(table);
			rhs->resolveSymbols(table);

		}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto lVal = lhs->emitValueCLLR(codeAsm);
			auto rVal = rhs->emitValueCLLR(codeAsm);

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

		void resolveSymbols(sptr<const SymbolTable> table) override {}

		cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_NULL, {}, {});
		}

	};

}
