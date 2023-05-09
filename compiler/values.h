
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

		virtual sptr<Token> firstTkn() const override
		{
			return lit;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return lit;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

			type = pType.resolve(table);

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
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

		FloatLiteralValue(ref<Token> l) : Value(ValueType::FLOAT_LITERAL), lit(&l) {}

		virtual sptr<Token> firstTkn() const override
		{
			return lit;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return lit;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

			type = pType.resolve(table);

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
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

		StringLitValue(ref<Token> str) : Value(ValueType::STR_LITERAL), lit(&str) {}

		virtual sptr<Token> firstTkn() const override
		{
			return lit;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return lit;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			auto sym = table->find("string");

			if (sym == nullptr)
			{
				return;
			}

			if (sym->type == SymbolType::TYPE)
			{
				type = (sptr<Type>)sym->data;
			}

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			type->emitDeclCLLR(codeAsm);

			auto sID = codeAsm.addString(lit->str);
			
			return codeAsm.pushNew(cllr::Opcode::VALUE_STR_LIT, { sID }, { type->id });
		}

	};

	struct BoolLitValue : public Value
	{
		const sptr<Token> lit;
		
		BoolLitValue(ref<Token> v) : Value(ValueType::STR_LITERAL), lit(&v)  {}

		virtual sptr<Token> firstTkn() const override
		{
			return lit;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return lit;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			auto sym = table->find("bool");

			if (sym == nullptr)
			{
				return;
			}

			if (sym->type == SymbolType::TYPE)
			{
				type = (sptr<Type>)sym->data;
			}

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
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

		virtual sptr<Token> firstTkn() const override
		{
			return start;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return end;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
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

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
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

		virtual sptr<Token> firstTkn() const override
		{
			return lValue->firstTkn();
		}

		virtual sptr<Token> lastTkn() const override
		{
			return rValue->lastTkn();
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return lValue->isCompileTimeConst() && rValue->isCompileTimeConst();
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			lValue->resolveSymbols(table);
			rValue->resolveSymbols(table);
			
			auto compat = lValue->type->isCompatible(op, rValue->type);

			//TODO check compatibility

			type = lValue->type;

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto lhs = lValue->emitValueCLLR(codeAsm);
			auto rhs = rValue->emitValueCLLR(codeAsm);

			auto cllrOp = cllr::Opcode::VALUE_EXPR;

			auto opType = opCategories.at(op);

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

		virtual sptr<Token> firstTkn() const override
		{
			return val->firstTkn();
		}

		virtual sptr<Token> lastTkn() const override
		{
			return chkPType->lastTkn();
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return val->isCompileTimeConst();
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			val->resolveSymbols(table);
			chkType = chkPType->resolve(table);

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
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

		virtual sptr<Token> firstTkn() const override
		{
			return array->firstTkn();
		}

		virtual sptr<Token> lastTkn() const override
		{
			return last;
		}

		virtual bool isLValue() const override
		{
			return true;
		}

		virtual bool isCompileTimeConst() const override
		{
			return array->isCompileTimeConst() && index->isCompileTimeConst();
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			array->resolveSymbols(table);
			index->resolveSymbols(table);

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
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

		virtual sptr<Token> firstTkn() const override
		{
			return lhs->firstTkn();
		}

		virtual sptr<Token> lastTkn() const override
		{
			return resultPType->lastTkn();
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return lhs->isCompileTimeConst();
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			type = resultPType->resolve(table);
			lhs->resolveSymbols(table);
			
		}
		
		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto vID = lhs->emitValueCLLR(codeAsm);

			auto loadID = codeAsm.pushNew(cllr::Opcode::VALUE_CAST, {}, { vID, type->id, 0 });

			return loadID;
		}

	};

	struct VarReadValue : public Value
	{
		const sptr<Token> var;

		sptr<Symbol> varSym = nullptr;

		VarReadValue(sptr<Token> v) : Value(ValueType::UNKNOWN), var(v) {}
		virtual ~VarReadValue() {}

		virtual sptr<Token> firstTkn() const override
		{
			return var;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return var;
		}

		virtual bool isLValue() const override
		{
			if (varSym == nullptr)
			{
				return true;
			}

			return varSym->type == SymbolType::VARIABLE;
		}

		virtual bool isCompileTimeConst() const override
		{
			return false;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			varSym = table->find(var->str);

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			if (varSym->type == SymbolType::VARIABLE)
			{
				return ((sptr<Variable>)varSym->data)->emitLoadCLLR(codeAsm, 0);
			}
			else if (varSym->type == SymbolType::VALUE)
			{
				return ((sptr<Value>)varSym->data)->emitValueCLLR(codeAsm);
			}

			return 0;
		}

	};

	struct MemberReadValue : public Value
	{
		uptr<Value> target = nullptr;
		sptr<Token> memberName = nullptr;
		sptr<Symbol> member = nullptr;

		MemberReadValue() : Value(ValueType::UNKNOWN) {}
		virtual ~MemberReadValue() {}

		virtual sptr<Token> firstTkn() const override
		{
			return target->firstTkn();
		}

		virtual sptr<Token> lastTkn() const override
		{
			return memberName;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return false;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			target->resolveSymbols(table);

			member = target->type->memberTable->find(memberName->str);

			if (member->type != SymbolType::VARIABLE)
			{
				//TODO complain
			}

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto vID = target->emitValueCLLR(codeAsm);

			return ((sptr<Variable>)member->data)->emitLoadCLLR(codeAsm, vID);
		}

	};
	
	struct UnaryValue : public Value
	{
		Operator op = Operator::UNKNOWN;
		sptr<Token> start = nullptr;
		uptr<Value> val = nullptr;
		sptr<Token> end = nullptr;

		UnaryValue() : Value(ValueType::UNKNOWN) {}
		virtual ~UnaryValue() {}

		virtual sptr<Token> firstTkn() const override
		{
			return start;
		}

		virtual sptr<Token> lastTkn() const override
		{
			if (end == nullptr)
				return val->lastTkn();
			return end;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return val->isCompileTimeConst();
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			val->resolveSymbols(table);
		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
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
		std::vector<uptr<ParsedType>> pGenerics;
		std::vector<sptr<Type>> generics;
		std::vector<uptr<Value>> args;
		sptr<Token> end = nullptr;

		FnCallValue() : Value(ValueType::FUNCTION_CALL) {}
		virtual ~FnCallValue() {}

		virtual sptr<Token> firstTkn() const override
		{
			if (target != nullptr)
				return target->firstTkn();
			return name;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return end;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			ptr<const SymbolTable> lookup = table.get();

			for (auto const& p : pGenerics)
			{
				generics.push_back(p->resolve(table));
			}

			if (target != nullptr)
			{
				args.push_back(target);

				lookup = target->type->memberTable.get();

			}

			for (auto const& a : args)
			{
				a->resolveSymbols(table);
			}

			auto fn = lookup->find(name->str);

			if (fn->type == SymbolType::FUNCTION)
			{
				funcID = 0;//lookup + create generic variant
			}
			else
			{
				//TODO complain
				return;
			}

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
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

		SetterValue() : Value(ValueType::UNKNOWN) {}
		virtual ~SetterValue() {}

		virtual sptr<Token> firstTkn() const override
		{
			return lhs->firstTkn();
		}

		virtual sptr<Token> lastTkn() const override
		{
			return rhs->firstTkn();
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override
		{
			lhs->resolveSymbols(table);
			rhs->resolveSymbols(table);

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto lVal = lhs->emitValueCLLR(codeAsm);
			auto rVal = rhs->emitValueCLLR(codeAsm);

			codeAsm.push(0, cllr::Opcode::ASSIGN, {}, { lVal, rVal });

			return 0;
		}

	};

	struct NullValue : public Value
	{
		const sptr<Token> lit;

		NullValue(sptr<Token> v) : Value(ValueType::UNKNOWN), lit(v) {}
		virtual ~NullValue() {}

		virtual sptr<Token> firstTkn() const override
		{
			return lit;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return lit;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(sptr<const SymbolTable> table) override {}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_NULL, {}, {});
		}

	};

}
