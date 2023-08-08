
#pragma once

#include "ast.h"
#include "fn.h"
#include "var.h"

#include "error.h"

namespace caliburn
{
	struct ShaderStage : ParsedObject
	{
		const ShaderType type;
		const uptr<ParsedFn> base;

		std::vector<sptr<ShaderIOVariable>> ios;
		
		ShaderStage(ref<uptr<ParsedFn>> fn) : base(std::move(fn)), type(SHADER_TYPES.find(base->name->str)->second) {}

		virtual ~ShaderStage() {}

		sptr<Token> firstTkn() const override
		{
			return base->first;
		}

		sptr<Token> lastTkn() const override
		{
			if (base->code == nullptr)
			{
				return base->retType->lastTkn();
			}

			return base->code->lastTkn();//idk
		}

		void prettyPrint(ref<std::stringstream> ss) const override {}

		uptr<Shader> compile(sptr<SymbolTable> table, sptr<const CompilerSettings> settings, ref<std::vector<sptr<Error>>> allErrs);

	};

	struct ShaderStatement : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<ShaderStage>> stages;

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
