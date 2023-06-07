
#pragma once

#include <vector>

#include "ast.h"
#include "typestruct.h"

namespace caliburn
{
	struct StructStatement : public GenericStatement
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;

		const sptr<Token> name;

		sptr<Type> innerType = nullptr;
		std::vector<sptr<Variable>> members;

		StructStatement(sptr<Token> n, StatementType type = StatementType::STRUCT) : GenericStatement(type), name(n) {}

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
			//innerType = std::make_unique<TypeStruct>(name->str, tNames.size() + cNames.size());
		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override {}

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
