
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
		const uptr<ParsedFn> base;

		ShaderType type = ShaderType::VERTEX;
		Token parentName;
		std::vector<sptr<ShaderIOVariable>> vtxInputs;

		ShaderStage(out<uptr<ParsedFn>> fn, in<Token> pname) : base(std::move(fn)), parentName(pname)
		{
			type = SHADER_TYPES.find(base->name.str)->second;

			for (auto const& a : base->args)
			{
				vtxInputs.push_back(new_sptr<ShaderIOVariable>(ShaderIOVarType::INPUT, a));

			}

		}
		virtual ~ShaderStage() = default;

		bool operator<(in<ShaderStage> other) const
		{
			return type < other.type;
		}

		Token firstTkn() const noexcept override
		{
			return base->code->firstTkn();
		}

		Token lastTkn() const noexcept override
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

	struct ShaderStmt : Statement
	{
		Token first;
		Token name;
		Token last;

		std::vector<uptr<ShaderStage>> stages;
		std::vector<std::pair<sptr<ParsedType>, Token>> descriptors;
		std::vector<sptr<ShaderIOVariable>> ioVars;

		ShaderStmt() : Statement(StmtType::SHADER) {}
		virtual ~ShaderStmt() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return last;
		}

		void prettyPrint(out<std::stringstream> ss) const override {}

		void declareHeader(sptr<SymbolTable> table, out<ErrorHandler> err) override {} //We don't add shaders to the symbol table

		void declareSymbols(sptr<SymbolTable> table, out<ErrorHandler> err) override {}

		void emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override {}

		void sortStages()
		{
			std::sort(stages.begin(), stages.end());
		}

		void compile(sptr<SymbolTable> table, sptr<CompilerSettings> settings, out<std::vector<uptr<Shader>>> shaders, out<std::vector<sptr<Error>>> compileErrs);

	};

}
