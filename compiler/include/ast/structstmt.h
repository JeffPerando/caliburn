
#pragma once

#include <map>

#include "ast.h"
#include "var.h"

#include "types/type.h"
#include "types/typestruct.h"

namespace caliburn
{
	struct StructStatement : Statement
	{
		const bool isConst;

		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Token> last = nullptr;
		sptr<TypeStruct> innerType = nullptr;
		sptr<GenericSignature> genSig = nullptr;

		std::map<std::string, Member> members;

		StructStatement(StatementType type = StatementType::STRUCT) : Statement(type),
			isConst(type == StatementType::RECORD) {}

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
				innerType = new_sptr<TypeStruct>(name->str, genSig, members);

			}

			table->add(name->str, innerType);

		}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;//Do not implement. We let the innermost RealType declare itself
		}

		void declareSymbols(sptr<SymbolTable> table) override {}

	};

}
