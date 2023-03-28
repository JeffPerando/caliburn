
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

		ShaderStageStatement(Statement* parent) : Statement(StatementType::SHADER, parent) {}

		cllr::CompilationUnit* compile()
		{
			auto codeAsm = cllr::Assembler();

			auto stdlib = new DefaultLib();
			auto symbols = stdlib->makeChild("code");

			code->declSymbols(*symbols);
			code->resolveSymbols(*symbols);

			code->emitDeclCLLR(codeAsm);

			delete stdlib;

			return new cllr::CompilationUnit{ cllr::Target::GPU, std::move(*codeAsm.getCode())};
		}

	};

	struct ShaderStatement : public Statement
	{
		std::vector<ShaderStageStatement*> stages;

		ShaderStatement(Statement* parent) : Statement(StatementType::SHADER, parent) {}
		virtual ~ShaderStatement() {}

		void compile(std::vector<cllr::CompilationUnit*>& codeDest, std::vector<uint32_t>* cbir)
		{
			for (auto stage : stages)
			{
				auto result = stage->compile();

				codeDest.push_back(result);

			}

			if (cbir != nullptr)
			{
				//TODO emit CBIR code
			}

		}

	};

}
