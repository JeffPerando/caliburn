
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
		const ShaderType type;
		const Token first, parentName;

		std::vector<sptr<ShaderIOVariable>> vtxInputs;

		ShaderStage(out<uptr<ParsedFn>> fn, in<Token> f, in<Token> pname) :
			base(std::move(fn)), first(f), parentName(pname), type(SHADER_TYPES.find(base->name.str)->second)
		{
			for (auto const& a : base->args)
			{
				vtxInputs.push_back(new_sptr<ShaderIOVariable>(a));

			}

		}

		virtual ~ShaderStage() = default;

		bool operator<(in<ShaderStage> other) const
		{
			return type < other.type;
		}

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return base->code->lastTkn();
		}

		void prettyPrint(out<std::stringstream> ss) const override {}

		uptr<Shader> compile(sptr<const CompilerSettings> settings, out<std::vector<std::string>> errs, in<TextDoc> doc, sptr<SymbolTable> table, std::vector<IOVar> inputs, out<std::vector<IOVar>> outputs);

	};

	struct ShaderStmt : Expr
	{
		Token first;
		Token name;
		Token last;

		std::vector<uptr<ShaderStage>> stages;
		std::vector<std::pair<sptr<ParsedType>, Token>> descriptors;
		std::vector<sptr<ShaderIOVariable>> ioVars;

		ShaderStmt() : Expr(ExprType::SHADER) {}

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

		ValueResult emitCodeCLLR(sptr<SymbolTable>, out<cllr::Assembler> codeAsm) const override
		{
			return ValueResult();
		}

		void compile(sptr<SymbolTable> table, sptr<CompilerSettings> settings, out<ShaderResult> result, in<TextDoc> doc);

	};

}
