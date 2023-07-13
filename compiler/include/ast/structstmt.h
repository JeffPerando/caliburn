
#pragma once

#include <vector>

#include "ast.h"
#include "var.h"

#include "types/type.h"
#include "types/typestruct.h"

namespace caliburn
{
	struct StructStatement : public Statement
	{
		const sptr<Token> name;

		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;
		sptr<BaseType> innerType = nullptr;
		sptr<GenericSignature> genSig = nullptr;

		std::vector<sptr<Variable>> members;
		std::vector<sptr<Function>> constructors, destructors;

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

		void declareHeader(sptr<SymbolTable> table) override
		{
			if (innerType == nullptr)
			{
				innerType = new_sptr<TypeStruct>(name->str, genSig);
			}

			table->add(name->str, innerType);

		}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;//Do not implement. We let the innermost RealType declare itself
		}

		void declareSymbols(sptr<SymbolTable> table) override {}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

	};

}
