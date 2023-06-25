
#pragma once

#include "ast.h"
#include "types/cbrn_std.h"
#include "cllr/cllropt.h"
#include "spirv/cllrspirv.h"

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
			type = SHADER_TYPES.find(n->str)->second;
		}
		ShaderStageStatement(ShaderType t) : Statement(StatementType::SHADER_STAGE), type(t), first(nullptr), name(nullptr) {}
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

		void declareHeader(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) {}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) {}

		void declareSymbols(sptr<SymbolTable> table) override
		{
			code->declareSymbols(table);
		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{

		}

		uptr<Shader> compile(sptr<SymbolTable> symbols, ptr<std::vector<uint32_t>> cbir, OptimizeLevel lvl)
		{
			auto codeAsm = cllr::Assembler(type);
			
			code->resolveSymbols(symbols, codeAsm);

			code->emitDeclCLLR(codeAsm);

			cllr::optimize(lvl, codeAsm);

			//TODO emit CBIR
			/*
			if (cbir)
			{
				code->emitCBIR(cbir);
			}
			*/

			cllr::SPIRVOutAssembler spirvAsm;

			auto spv = spirvAsm.translateCLLR(codeAsm, *codeAsm.getCode());

			auto out = new_uptr<Shader>();
			
			out->type = type;
			out->spirv = std::move(spv);

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

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void prettyPrint(ref<std::stringstream> ss) const override {}

		void declareHeader(sptr<SymbolTable> table) const override {} //We don't add shaders to the symbol table

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) {}

		void declareSymbols(sptr<SymbolTable> table) override {}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			for (auto const& stage : stages)
			{
				stage->resolveSymbols(table, codeAsm);

			}

		}

	};

}
