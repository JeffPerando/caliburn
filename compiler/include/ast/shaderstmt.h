
#pragma once

#include <algorithm>

#include "ast.h"
#include "fn.h"
#include "var.h"

#include "error.h"

namespace caliburn
{
	struct ShaderStage : ParsedObject
	{
		ShaderType type = ShaderType::VERTEX;

		const uptr<ParsedFn> base;

		sptr<Token> parentName;

		std::vector<sptr<ShaderIOVariable>> vtxInputs;

		ShaderStage(out<uptr<ParsedFn>> fn, sptr<Token> pname) : base(std::move(fn)), parentName(pname)
		{
			type = SHADER_TYPES.find(base->name->str)->second;

			for (auto const& a : base->args)
			{
				vtxInputs.push_back(new_sptr<ShaderIOVariable>(ShaderIOVarType::INPUT, *a));

			}

		}
		virtual ~ShaderStage() {}

		bool operator<(in<ShaderStage> other) const
		{
			return type < other.type;
		}

		sptr<Token> firstTkn() const override
		{
			return base->first;
		}

		sptr<Token> lastTkn() const override
		{
			if (base->code == nullptr)
			{
				return base->returnType->lastTkn();
			}

			return base->code->lastTkn();//idk
		}

		void prettyPrint(out<std::stringstream> ss) const override {}

		uptr<Shader> compile(sptr<SymbolTable> table, sptr<const CompilerSettings> settings, out<std::vector<sptr<Error>>> allErrs);

	};

	struct ShaderStatement : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<ShaderStage>> stages;

		std::vector<std::pair<sptr<ParsedType>, sptr<Token>>> descriptors;

		std::vector<sptr<ShaderIOVariable>> ioVars;

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

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

		void sortStages()
		{
			std::sort(stages.begin(), stages.end());
		}

		void compile(sptr<SymbolTable> table, sptr<CompilerSettings> settings, out<std::vector<uptr<Shader>>> shaders, out<std::vector<sptr<Error>>> compileErrs);

	};

}
