
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
		const Token first;
		const Token name;

		Token last;

		sptr<TypeStruct> innerType = nullptr;
		uptr<GenericSignature> genSig = nullptr;
		std::map<std::string_view, sptr<ParsedVar>> members;
		std::vector<uptr<ParsedFn>> memberFns;

		StructStmt(StmtType type, in<Token> f, in<Token> n) :
			Statement(type), isConst(type == StmtType::RECORD), first(f), name(n) {}

		virtual ~StructStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return last;
		}

		void declareHeader(sptr<SymbolTable> table, out<ErrorHandler> err) override
		{
			if (!table->add(name.str, new_sptr<TypeStruct>(name.str, genSig, members, memberFns)))
			{
				//TODO complain
			}

		}

		//Do not implement. We let the innermost type declare itself
		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

	};

}
