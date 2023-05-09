
#pragma once

#include "ast.h"

namespace caliburn
{
	struct ImportStatement : public Statement
	{
		const sptr<Token> first;
		sptr<Token> name = nullptr;
		sptr<Token> alias = nullptr;

		ImportStatement(sptr<Token> f) : Statement(StatementType::IMPORT), first(f) {}
		virtual ~ImportStatement() {}

		virtual sptr<Token> firstTkn() const override
		{
			return first;
		}

		virtual sptr<Token> lastTkn() const override
		{
			if (alias == nullptr)
			{
				return name;
			}

			return alias;
		}

		virtual void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		virtual void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) {}

	};

	struct ModuleStatement : public Statement
	{
		const sptr<Token> first, name;

		ModuleStatement(sptr<Token> s, sptr<Token> n) : Statement(StatementType::MODULE), first(s), name(n) {}
		virtual ~ModuleStatement() {}

		virtual sptr<Token> firstTkn() const override
		{
			return first;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return name;
		}

		virtual void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		virtual void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) {}

	};

}
