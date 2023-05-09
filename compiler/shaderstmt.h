
#pragma once

#include "ast.h"
#include "cbrn_std.h"
#include "cllrasm.h"

namespace caliburn
{
	struct ShaderStageStatement : public Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;

		std::set<std::pair<sptr<Token>, uptr<ParsedType>>> inputs;
		uptr<ParsedType> output = nullptr;

		uptr<ScopeStatement> code = nullptr;

		ShaderStageStatement() : Statement(StatementType::SHADER_STAGE) {}
		/*
		uptr<cllr::CompilationUnit> compile(sptr<SymbolTable> symbols)
		{
			auto codeAsm = cllr::Assembler();
			
			code->declareSymbols(symbols, codeAsm);
			code->resolveSymbols(symbols, codeAsm);

			code->emitDeclCLLR(codeAsm);

			return std::make_unique<cllr::CompilationUnit>(cllr::Target::GPU, std::move(*codeAsm.getCode()));
		}
		*/
	};

	struct ShaderStatement : public Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<ShaderStageStatement>> stages;

		ShaderStatement() : Statement(StatementType::SHADER) {}
		virtual ~ShaderStatement() {}
		/*
		void compile(sptr<SymbolTable> table, ref<std::vector<uptr<cllr::CompilationUnit>>> codeDest, ptr<std::vector<uint32_t>> cbir)
		{
			for (auto const& stage : stages)
			{
				auto result = stage->compile(table);

				codeDest.push_back(std::move(result));

			}

			if (cbir != nullptr)
			{
				//TODO emit CBIR code
			}

		}
		*/
	};

}
