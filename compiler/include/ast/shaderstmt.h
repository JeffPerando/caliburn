
#pragma once

#include "ast.h"
#include "var.h"

#include "error.h"

namespace caliburn
{
	struct ShaderStageStatement : Statement
	{
		const sptr<ErrorHandler> errors;
		const sptr<Token> first;
		const sptr<Token> name;

		uptr<ScopeStatement> code = nullptr;

		ShaderType type = ShaderType::COMPUTE;

		std::vector<sptr<ShaderIOVariable>> ios;

		sptr<ParsedType> retType = nullptr;

		ShaderStageStatement(sptr<ErrorHandler> err, sptr<Token> f, sptr<Token> n) : Statement(StatementType::SHADER_STAGE),
			errors(err), first(f), name(n)
		{
			type = SHADER_TYPES.find(n->str)->second;
		}
		ShaderStageStatement(ShaderType t) : Statement(StatementType::SHADER_STAGE),
			type(t), first(nullptr), name(nullptr) {}
		virtual ~ShaderStageStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return code->lastTkn();//idk
		}

		void prettyPrint(ref<std::stringstream> ss) const override {}

		//void declareHeader(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

		void declareSymbols(sptr<SymbolTable> table) override
		{
			code->declareSymbols(table);
		}

		uptr<Shader> compile(sptr<SymbolTable> table, OptimizeLevel lvl);

	};

	struct ShaderStatement : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<ShaderStageStatement>> stages;

		std::vector<std::pair<sptr<ParsedType>, sptr<Token>>> descriptors;

		ShaderStatement() : Statement(StatementType::SHADER) {}
		virtual ~ShaderStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void prettyPrint(ref<std::stringstream> ss) const override {}

		void declareHeader(sptr<SymbolTable> table) override {} //We don't add shaders to the symbol table

		void declareSymbols(sptr<SymbolTable> table) override {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			return 0;
		}

	};

}
