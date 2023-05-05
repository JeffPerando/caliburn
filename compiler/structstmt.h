
#pragma once

#include <vector>

#include "ast.h"
#include "typestruct.h"

namespace caliburn
{
	struct StructStatement : public GenericStatement
	{
		ptr<Token> first = nullptr;
		ptr<Token> last = nullptr;

		const ptr<Token> name;

		ptr<Type> innerType = nullptr;
		std::vector<ptr<Variable>> members;

		StructStatement(ptr<Token> n, StatementType type = StatementType::STRUCT) : GenericStatement(type), name(n) {}

		virtual ~StructStatement()
		{
			if (innerType != nullptr)
			{
				delete innerType;
				innerType = nullptr;

			}

		}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			return last;
		}

		//Only used by top-level statements which declare symbols. The rest, like local variables, should use declareSymbols() instead
		virtual void declareHeader(ref<SymbolTable> table, cllr::Assembler& codeAsm)
		{
			innerType = new TypeStruct(name->str, tNames.size() + cNames.size());
		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override {}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override
		{
			for (auto mem : members)
			{
				
			}

		}

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override
		{
			for (auto mem : members)
			{
				mem->resolveSymbols(table);

			}

		}

	};

}
