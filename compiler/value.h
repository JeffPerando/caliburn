
#pragma once

#include "cllr.h"
#include "type.h"

namespace caliburn
{
	struct ConstantValue : public Value
	{
		Token* lit = nullptr;

		ConstantValue() : Value(ValueType::LITERAL) {}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual void resolveSymbols(SymbolTable& const table) override
		{
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

			type = pType.resolve(table);

			if (type == nullptr)
			{
				//TODO complain
			}

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			type->emitDeclCLLR(codeAsm);
		}

	};

	struct ExpressionValue : public Value
	{
		Value* lValue = nullptr;
		Value* rValue = nullptr;
		Operator op = Operator::UNKNOWN;

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual void resolveSymbols(SymbolTable& table) override
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

			type->emitDeclCLLR(codeAsm);

			lValue->emitDeclCLLR(codeAsm);
			rValue->emitDeclCLLR(codeAsm);

		}

	};

	struct CastValue : public Value
	{
		cllr::SSA id = 0;
		Value* lValue = nullptr;
		ParsedType* resultPType = nullptr;

		CastValue(Value* lhs, ParsedType* result) : Value(ValueType::EXPRESSION), lValue(lhs), resultPType(result) {}
		virtual ~CastValue() {}

		virtual bool isLValue() const override
		{
			return lValue == nullptr ? false : lValue->isLValue();
		}

		virtual void resolveSymbols(SymbolTable& const table) override
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