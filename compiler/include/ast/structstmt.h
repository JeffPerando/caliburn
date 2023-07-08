
#pragma once

#include <vector>

#include "ast.h"
#include "var.h"

#include "types/typestruct.h"

namespace caliburn
{
	struct StructStatement : public Statement
	{
		const sptr<Token> name;

		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;
		sptr<BaseType> innerType = nullptr;
		std::vector<sptr<Variable>> members;

		StructStatement(sptr<Token> n, StatementType type = StatementType::STRUCT) : Statement(type), name(n) {}

		virtual ~StructStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		virtual void declareHeader(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
		{
			//innerType = new_uptr<TypeStruct>(name->str, tNames.size() + cNames.size());
		}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

		void declareSymbols(sptr<SymbolTable> table) override
		{
			for (auto const& mem : members)
			{
				
			}

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			for (auto const& mem : members)
			{
				mem->resolveSymbols(table);

			}

		}

	};

}
