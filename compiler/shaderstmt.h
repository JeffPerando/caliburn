
#pragma once

#include "ast.h"
#include "cbrn_std.h"
#include "cllrasm.h"
#include "cllrspirv.h"

namespace caliburn
{
	struct ShaderStageStatement : public Statement
	{
		const sptr<Token> first;
		const sptr<Token> name;

		uptr<ScopeStatement> code = nullptr;

		ShaderType type = ShaderType::COMPUTE;

		std::vector<std::pair<sptr<Token>, uptr<ParsedType>>> inputsParsed;
		std::vector<std::pair<sptr<Token>, sptr<Type>>> inputs;

		uptr<ParsedType> retTypeParsed = nullptr;
		sptr<Type> retType = nullptr;

		ShaderStageStatement(sptr<Token> f, sptr<Token> n) : Statement(StatementType::SHADER_STAGE), first(f), name(n)
		{
			type = shaderTypes.find(n->str)->second;
		}
		ShaderStageStatement(ShaderType t) : Statement(StatementType::SHADER_STAGE), type(t), first(nullptr), name(nullptr) {}
		virtual ~ShaderStageStatement() {}

		uptr<Shader> compile(sptr<SymbolTable> symbols, ptr<std::vector<uint32_t>> cbir)
		{
			auto codeAsm = cllr::Assembler();
			
			code->declareSymbols(symbols, codeAsm);
			code->resolveSymbols(symbols, codeAsm);

			code->emitDeclCLLR(codeAsm);

			//TODO emit CBIR
			/*
			if (cbir)
			{
				code->emitCBIR(cbir);
			}
			*/

			auto spirvAsm = cllr::SPIRVOutAssembler();

			auto spv = spirvAsm.translateCLLR(*(codeAsm.getCode()));

			auto out = std::make_unique<Shader>();
			
			out->type = type;
			out->spirv = *spv;

			if (type == ShaderType::VERTEX)
			{
				for (uint32_t i = 0; i < inputs.size(); ++i)
				{
					auto const& name = inputs.at(i).first->str;

					out->inputs.push_back(VertexInputAttribute{name, i});

				}

			}

			return out;
		}
		
	};

	struct ShaderStatement : public Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<ShaderStageStatement>> stages;

		std::vector<std::pair<uptr<ParsedType>, sptr<Token>>> descriptorsParsed;
		std::vector<std::pair<sptr<Type>, sptr<Token>>> descriptors;

		ShaderStatement() : Statement(StatementType::SHADER) {}
		virtual ~ShaderStatement() {}
		
		void compile(sptr<SymbolTable> table, ref<std::vector<uptr<Shader>>> codeDest, ptr<std::vector<uint32_t>> cbir)
		{
			for (auto const& stage : stages)
			{
				auto result = stage->compile(table, cbir);

				for (uint32_t i = 0; i < descriptors.size(); ++i)
				{
					auto const& desc = descriptors.at(i);

					result->sets.push_back(DescriptorSet{desc.second->str, i});

				}

				codeDest.push_back(std::move(result));

			}

			if (cbir != nullptr)
			{
				//TODO emit CBIR code
			}

		}
		
	};

}
