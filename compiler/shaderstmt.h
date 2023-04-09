
#pragma once

#include "ast.h"
#include "cbrn_std.h"
#include "cllrasm.h"

namespace caliburn
{
	struct ShaderStageStatement : public Statement
	{
		Token* first = nullptr;
		Token* name = nullptr;

		std::set<std::pair<Token*, ParsedType*>> inputs;
		ParsedType* output = nullptr;

		ScopeStatement* code = nullptr;

		ShaderStageStatement() : Statement(StatementType::SHADER_STAGE) {}

		cllr::CompilationUnit* compile(ref<SymbolTable> symbols)
		{
			auto codeAsm = cllr::Assembler();
			
			code->declareSymbols(symbols, codeAsm);
			code->resolveSymbols(symbols, codeAsm);

			code->emitDeclCLLR(codeAsm);

			return new cllr::CompilationUnit{ cllr::Target::GPU, std::move(*codeAsm.getCode())};
		}

	};

	struct ShaderStatement : public Statement
	{
		std::vector<ShaderStageStatement*> stages;

		ShaderStatement() : Statement(StatementType::SHADER) {}
		virtual ~ShaderStatement() {}

		void compile(ref<SymbolTable> table, std::vector<cllr::CompilationUnit*>& codeDest, std::vector<uint32_t>* cbir)
		{
			for (auto stage : stages)
			{
				auto result = stage->compile(table);

				codeDest.push_back(result);

			}

			if (cbir != nullptr)
			{
				//TODO emit CBIR code
			}

		}

	};

}
