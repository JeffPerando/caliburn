
#pragma once

#include "cllr.h"
#include "type.h"

namespace caliburn
{
	struct ConstantValue : public Value
	{
		Token* const lit;

		ConstantValue(Token* l) : Value(ValueType::LITERAL), lit(l)
		{
			if (type == nullptr)
			{
				//TODO complain
			}

		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const
		{
			return true;
		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

			type = pType.resolve(table);

			//TODO parse

		}

		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			id = codeAsm.createSSA(cllr::Opcode::VALUE_LITERAL);
		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			type->emitDeclCLLR(codeAsm);

			codeAsm.push(id, cllr::Opcode::VALUE_LITERAL, { type->id, 0, 0 });

		}

		virtual cllr::SSA emitLoadCLLR(cllr::Assembler& codeAsm)
		{
			if (id == 0)
			{
				emitDeclCLLR(codeAsm);
			}

			return id;
		}

		virtual void emitStoreCLLR(cllr::Assembler& codeAsm, cllr::SSA value) {}

	};

	struct ExpressionValue : public Value
	{
		Value* const lValue;
		Value* const rValue;
		Operator const op;

		ExpressionValue(Value* l, Value* r, Operator op) : Value(ValueType::EXPRESSION), lValue(l), rValue(r), op(op) {}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst()
		{
			return lValue->isCompileTimeConst() && rValue->isCompileTimeConst();
		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			if (lValue == nullptr || rValue == nullptr)
			{
				//TODO complain
				return;
			}

			lValue->resolveSymbols(table);
			rValue->resolveSymbols(table);

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			if (lValue == nullptr || rValue == nullptr)
			{
				//TODO complain
				return;
			}

			id = codeAsm.createSSA(cllr::Opcode::VALUE_EXPR);

			type->emitDeclCLLR(codeAsm);

			lValue->emitDeclCLLR(codeAsm);
			rValue->emitDeclCLLR(codeAsm);

		}

		virtual cllr::SSA emitLoadCLLR(cllr::Assembler& codeAsm)
		{
			auto lhs = lValue->emitLoadCLLR(codeAsm);
			auto rhs = rValue->emitLoadCLLR(codeAsm);

			codeAsm.push(id, cllr::Opcode::VALUE_EXPR, { (uint32_t)op, lhs, rhs });
		}

		virtual void emitStoreCLLR(cllr::Assembler& codeAsm, cllr::SSA value) {}

	};

	struct SubArrayValue : public Value
	{
		Value* const array;
		Value* const index;

		SubArrayValue(Value* a, Value* i) : Value(ValueType::VARIABLE), array(i), index(i)
		{
			if (!array || !index)
			{
				//TODO complain
			}

		}

		virtual bool isLValue() const override
		{
			return true;
		}

		virtual bool isCompileTimeConst() const
		{
			return array->isCompileTimeConst() && index->isCompileTimeConst();
		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			array->resolveSymbols(table);
			index->resolveSymbols(table);

		}

		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			array->getSSAs(codeAsm);
			index->getSSAs(codeAsm);

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override {}

		virtual cllr::SSA emitLoadCLLR(cllr::Assembler& codeAsm)
		{
			auto aID = array->emitLoadCLLR(codeAsm);
			auto iID = index->emitLoadCLLR(codeAsm);

			auto loadID = codeAsm.push(0, cllr::Opcode::VALUE_SUBARRAY, { aID, iID, 0 }, true);

			return loadID;
		}

		virtual void emitStoreCLLR(cllr::Assembler& codeAsm, cllr::SSA value)
		{
			auto loadID = emitLoadCLLR(codeAsm);

			codeAsm.push(0, cllr::Opcode::ASSIGN_ARRAY, { loadID, value });

		}

	};

	struct CastValue : public Value
	{
		Value* const lValue;
		ParsedType* const resultPType;

		CastValue(Value* lhs, ParsedType* result) : Value(ValueType::EXPRESSION), lValue(lhs), resultPType(result) {}
		virtual ~CastValue() {}

		virtual bool isLValue() const override
		{
			return lValue == nullptr ? false : lValue->isLValue();
		}

		virtual bool isCompileTimeConst() const
		{
			return lValue->isCompileTimeConst();
		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			if (lValue == nullptr || resultPType == nullptr)
			{
				//TODO complain
				return;
			}

			lValue->resolveSymbols(table);
			type = resultPType->resolve(table);

		}

		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			if (lValue == nullptr)
			{
				return;
			}

			type->getSSAs(codeAsm);
			lValue->getSSAs(codeAsm);
			id = codeAsm.createSSA(cllr::Opcode::VALUE_CAST);

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			if (lValue == nullptr)
			{
				//TODO complain
				return;
			}

			type->emitDeclCLLR(codeAsm);
			lValue->emitDeclCLLR(codeAsm);

		}

	};

}