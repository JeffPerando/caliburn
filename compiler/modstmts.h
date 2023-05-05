
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ImportStatement : public Statement
	{
		const ptr<Token> first;
		Token* name = nullptr;
		Token* alias = nullptr;

		ImportStatement(ptr<Token> f) : Statement(StatementType::IMPORT), first(f) {}
		virtual ~ImportStatement() {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			if (alias == nullptr)
			{
				return name;
			}

			return alias;
		}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override {}

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override {}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) {}

	};

	struct ModuleStatement : public Statement
	{
		const ptr<Token> first, name;

		ModuleStatement(ptr<Token> s, ptr<Token> n) : Statement(StatementType::MODULE), first(s), name(n) {}
		virtual ~ModuleStatement() {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			return name;
		}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override {}

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override {}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) {}

	};

}
