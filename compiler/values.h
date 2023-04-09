
#pragma once

#include "basic.h"
#include "cllr.h"
#include "type.h"

namespace caliburn
{
	struct LiteralValue : public Value
	{
		const ptr<Token> lit;

		//TODO replace with 64-bit
		uint32_t litBits = 0;

		LiteralValue(ptr<Token> l) : Value(ValueType::LITERAL), lit(l)
		{
			if (type == nullptr)
			{
				//TODO complain
			}

		}

		virtual Token* firstTkn() const override
		{
			return lit;
		}

		virtual Token* lastTkn() const override
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

		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

			type = pType.resolve(table);

			//TODO parse token

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) override
		{
			type->emitDeclCLLR(codeAsm);

			id = codeAsm.push(0, cllr::Opcode::VALUE_LITERAL, { litBits }, { type->id }, true);

			return id;
		}

	};

	struct StringLitValue : public Value
	{
		ptr<Token> lit;

		StringLitValue() : Value(ValueType::STR_LITERAL) {}

		virtual Token* firstTkn() const override
		{
			return lit;
		}

		virtual Token* lastTkn() const override
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

		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			auto sym = table.find("string");

			if (sym == nullptr)
			{
				return;
			}

			if (sym->type == SymbolType::TYPE)
			{
				type = (ConcreteType*)sym->data;
			}

		}

		virtual cllr::SSA emitValueCLLR(cllr::Assembler& codeAsm) override
		{
			type->emitDeclCLLR(codeAsm);

			auto sID = codeAsm.addString(lit->str);
			id = codeAsm.push(0, cllr::Opcode::VALUE_LITERAL, { sID }, { type->id }, true);

			return id;
		}

	};

	struct ExpressionValue : public Value
	{
		ptr<Value> lValue = nullptr;
		ptr<Value> rValue = nullptr;
		Operator op = Operator::UNKNOWN;

		ExpressionValue() : Value(ValueType::EXPRESSION) {}

		virtual Token* firstTkn() const override
		{
			return lValue->firstTkn();
		}

		virtual Token* lastTkn() const override
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

		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			if (lValue == nullptr || rValue == nullptr)
			{
				//TODO complain
				return;
			}

			lValue->resolveSymbols(table);
			rValue->resolveSymbols(table);

		}

		virtual cllr::SSA emitValueCLLR(cllr::Assembler& codeAsm) override
		{
			if (lValue == nullptr || rValue == nullptr)
			{
				//TODO complain
				return 0;
			}

			auto lhs = lValue->emitValueCLLR(codeAsm);
			auto rhs = rValue->emitValueCLLR(codeAsm);

			id = codeAsm.push(id, cllr::Opcode::VALUE_EXPR, { (uint32_t)op }, { lhs, rhs });

			type = lValue->type;

			return id;
		}

	};

	struct SubArrayValue : public Value
	{
		ptr<Value> array;
		ptr<Value> index;
		ptr<Token> last;

		SubArrayValue() : Value(ValueType::SUB_ARRAY) {}

		virtual Token* firstTkn() const override
		{
			return array->firstTkn();
		}

		virtual Token* lastTkn() const override
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

		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			array->resolveSymbols(table);
			index->resolveSymbols(table);

		}

		virtual cllr::SSA emitValueCLLR(cllr::Assembler& codeAsm) override
		{
			auto aID = array->emitValueCLLR(codeAsm);
			auto iID = index->emitValueCLLR(codeAsm);

			auto loadID = codeAsm.push(0, cllr::Opcode::VALUE_SUBARRAY, {}, { aID, iID }, true);

			return loadID;
		}

	};

	struct CastValue : public Value
	{
		ptr<Value> lhs;
		ptr<ParsedType> resultPType;

		CastValue() : Value(ValueType::CAST) {}
		virtual ~CastValue() {}

		virtual Token* firstTkn() const override
		{
			return lhs->firstTkn();
		}

		virtual Token* lastTkn() const override
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

		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			type = resultPType->resolve(table);
			lhs->resolveSymbols(table);
			
		}
		
		virtual cllr::SSA emitValueCLLR(cllr::Assembler& codeAsm) override
		{
			auto vID = lhs->emitValueCLLR(codeAsm);

			auto loadID = codeAsm.push(0, cllr::Opcode::VALUE_CAST, {}, { vID, type->id, 0 }, true);

			return loadID;
		}

	};

	struct FuncCallValue : public Value
	{
		ptr<Token> name = nullptr;
		std::vector<Value*> args;
		ptr<Token> last = nullptr;

		cllr::SSA funcID = 0;

		FuncCallValue() : Value(ValueType::FUNCTION_CALL) {}
		virtual ~FuncCallValue() {}

		virtual Token* firstTkn() const override
		{
			return name;
		}

		virtual Token* lastTkn() const override
		{
			return last;
		}

		virtual bool isLValue() const override
		{
			return false;
		}

		virtual bool isCompileTimeConst() const override
		{
			return false;
		}

		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			//funcID = table.getFunction(name->str);

			for (auto arg : args)
			{
				arg->resolveSymbols(table);

			}

		}

		virtual cllr::SSA emitValueCLLR(cllr::Assembler& codeAsm) override
		{
			std::vector<cllr::SSA> vals;

			vals.reserve(args.size());

			for (auto arg : args)
			{
				vals.push_back(arg->emitValueCLLR(codeAsm));
			}

			auto vID = codeAsm.push(0, cllr::Opcode::CALL, { (uint32_t)args.size() }, { funcID }, true);

			for (uint32_t i = 0; i < vals.size(); ++i)
			{
				codeAsm.push(0, cllr::Opcode::CALL_ARG, { i }, { vID, vals.at(i) });
			}

			return vID;
		}

	};

	struct VarReadValue : public Value
	{
		const ptr<Token> varTkn;
		Symbol* varSym = nullptr;

		VarReadValue(Token* tkn) : varTkn(tkn), Value(ValueType::VAR_READ) {}
		~VarReadValue() {}

		virtual Token* firstTkn() const override
		{
			return varTkn;
		}

		virtual Token* lastTkn() const override
		{
			return varTkn;
		}

		virtual bool isLValue() const override
		{
			return true;
		}

		/*
		TECHNICALLY, if we replace the value of a variable with a constant, then this should
		reflect that replacement and return true. However, we don't know when this can be
		called. So, we just return false and play it safe.
		*/
		virtual bool isCompileTimeConst() const override
		{
			return false;
		}

		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			varSym = table.find(varTkn->str);

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) override
		{
			if (varSym == nullptr)
			{
				//TODO complain
				return 0;
			}

			if (varSym->type == SymbolType::VARIABLE)
			{
				id = ((Variable*)varSym->data)->emitLoadCLLR(codeAsm);
			}
			else if (varSym->type == SymbolType::VALUE)
			{
				auto val = (Value*)varSym->data;
				val->emitValueCLLR(codeAsm);
				id = val->id;
			}
			else
			{
				//TODO complain
				return 0;
			}

			return id;
		}

	};

	struct DefaultInitValue : public Value
	{
		const ptr<ParsedType> pType;

		DefaultInitValue(ParsedType* pt) : Value(ValueType::DEFAULT_INIT), pType(pt) {}
		virtual ~DefaultInitValue() {}

		//Welp. This isn't actually a parsed value, go figure.
		virtual Token* firstTkn() const override
		{
			return nullptr;
		}

		virtual Token* lastTkn() const override
		{
			return nullptr;
		}

		virtual bool isLValue() const override
		{
			return true;
		}

		virtual bool isCompileTimeConst() const override
		{
			return true;
		}

		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			type = pType->resolve(table);

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) override
		{
			return type->emitDefaultInitValue(codeAsm);
		}

	};

}
