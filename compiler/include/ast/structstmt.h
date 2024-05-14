
#pragma once

#include <map>

#include "ast/ast.h"
#include "ast/basetypes.h"
#include "ast/var.h"

namespace caliburn
{
	struct StructStmt : Statement
	{
		const bool isConst;

		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Token> last = nullptr;
		sptr<TypeStruct> innerType = nullptr;
		
		uptr<GenericSignature> genSig = nullptr;

		std::map<std::string_view, sptr<ParsedVar>> members;
		std::vector<uptr<ParsedFn>> memberFns;

		StructStmt(StmtType type = StmtType::STRUCT) :
			Statement(type), isConst(type == StmtType::RECORD) {}

		virtual ~StructStmt() = default;

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void declareHeader(sptr<SymbolTable> table, out<ErrorHandler> err) override
		{
			if (!table->add(name->str, new_sptr<TypeStruct>(name->str, genSig, members, memberFns)))
			{
				//TODO complain
			}

		}

		//Do not implement. We let the innermost type declare itself
		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

	};

}
