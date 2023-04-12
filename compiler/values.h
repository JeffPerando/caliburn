
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

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			type->emitDeclCLLR(codeAsm);

			return codeAsm.pushNew(cllr::Opcode::VALUE_LITERAL, { litBits }, { type->id });
		}

	};

	struct StringLitValue : public Value
	{
		ptr<Token> lit = nullptr;

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
				type = (Type*)sym->data;
			}

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			type->emitDeclCLLR(codeAsm);

			auto sID = codeAsm.addString(lit->str);
			
			return codeAsm.pushNew(cllr::Opcode::VALUE_LITERAL, { sID }, { type->id });
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
			lValue->resolveSymbols(table);
			rValue->resolveSymbols(table);
			
			auto compat = lValue->type->isCompatible(op, rValue->type);

			//bla bla bla

			type = lValue->type;

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto lhs = lValue->emitValueCLLR(codeAsm);
			auto rhs = rValue->emitValueCLLR(codeAsm);

			return codeAsm.pushNew(cllr::Opcode::VALUE_EXPR, { (uint32_t)op }, { lhs, rhs });
		}

	};

	struct SubArrayValue : public Value
	{
		ptr<Value> array = nullptr;
		ptr<Value> index = nullptr;
		ptr<Token> last = nullptr;

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
		ptr<Value> lhs = nullptr;
		ptr<ParsedType> resultPType = nullptr;

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
		
		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			auto vID = lhs->emitValueCLLR(codeAsm);

			auto loadID = codeAsm.pushNew(cllr::Opcode::VALUE_CAST, {}, { vID, type->id, 0 });

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

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			std::vector<cllr::SSA> vals;

			vals.reserve(args.size());

			for (auto arg : args)
			{
				vals.push_back(arg->emitValueCLLR(codeAsm));
			}

			auto vID = codeAsm.pushNew(cllr::Opcode::CALL, { (uint32_t)args.size() }, { funcID });

			for (uint32_t i = 0; i < vals.size(); ++i)
			{
				codeAsm.push(0, cllr::Opcode::CALL_ARG, { i }, { vID, vals.at(i) });
			}

			return vID;
		}

	};

	struct SymbolLookupValue : public Value
	{
		const ptr<Token> varTkn;
		Symbol* varSym = nullptr;

		SymbolLookupValue(Token* tkn) : varTkn(tkn), Value(ValueType::VAR_READ) {}
		virtual ~SymbolLookupValue() {}

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

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			if (varSym == nullptr)
			{
				//TODO complain
				return 0;
			}

			if (varSym->type == SymbolType::VARIABLE)
			{
				return ((Variable*)varSym->data)->emitLoadCLLR(codeAsm);
			}
			else if (varSym->type == SymbolType::VALUE)
			{
				auto val = (Value*)varSym->data;
				return val->emitValueCLLR(codeAsm);
			}
			
			//TODO complain
			return 0;
		}

	};

	struct AccessChainValue : public Value
	{
		std::vector<ptr<Token>> accesses;
		ptr<Symbol> endSym = nullptr;
		
		AccessChainValue() : Value(ValueType::UNKNOWN) {}
		virtual ~AccessChainValue() {}

		virtual Token* firstTkn() const override
		{
			return accesses.front();
		}

		virtual Token* lastTkn() const override
		{
			return accesses.back();
		}

		virtual bool isLValue() const override
		{
			return true;
		}

		virtual bool isCompileTimeConst() const override
		{
			return false;
		}

		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!! THIS CODE IS STUPID AND DOES NOT WORK !!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		virtual void resolveSymbols(ref<const SymbolTable> table) override
		{
			auto lookup = &table;
			size_t depth = 0;

			/*
			We need to change this code to account for members existing.

			So a Type.getMember() method needs to be written.
			*/
			for (auto tkn : accesses)
			{
				++depth;
				
				auto sym = lookup->find(tkn->str);

				if (sym == nullptr)
				{
					//TODO complain
					break;
				}

				switch (sym->type)
				{
				case SymbolType::MODULE: lookup = ((SymbolTable*)sym->data); continue;
				case SymbolType::VARIABLE: lookup = ((Variable*)sym->data)->type->members; continue;
				default: endSym = sym; break;
				}

				//Workaround for inability to break a loop within a switch (yes, that's a thing)
				if (endSym != nullptr)
				{
					break;
				}

			}

		}

		virtual cllr::SSA emitValueCLLR(ref<cllr::Assembler> codeAsm) const override
		{
			if (endSym->type == SymbolType::VALUE)
			{
				return ((Value*)endSym->data)->emitValueCLLR(codeAsm);
			}

			if (endSym->type == SymbolType::VARIABLE)
			{
				return ((Variable*)endSym->data)->emitLoadCLLR(codeAsm);
			}

			return 0;
		}

	};

}
