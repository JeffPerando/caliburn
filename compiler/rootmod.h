
#pragma once

#include <map>
#include <string>
#include <vector>

#include "ast.h"
#include "funcstmt.h"
#include "module.h"
#include "shaderstmt.h"

namespace caliburn
{
	struct RootModule : public Module
	{
		std::vector<std::pair<std::string, std::string>> modRequests;
		std::map<std::string, Module*> modules;
		
		RootModule() {}
		virtual ~RootModule() {}

		void importModule(std::string name, std::string alias = "")
		{
			if (alias == "")
			{
				alias = name;
			}

			//swap the args so the alias is the one acting as the 'key' (also matches with the map sig)
			modRequests.push_back(std::pair(alias, name));
		}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override
		{

		}

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override
		{

		}

		virtual bool validateModule() const override
		{
			return false;
		}

	};

	struct CompiledModule : public Module
	{
		std::string name;

		std::map<std::string, Variable*> vars;
		std::map<std::string, Type*> types;
		std::map<std::string, FunctionStatement*> fns;
		std::map<std::string, ShaderStatement*> shaders;

		CompiledModule() {}
		virtual ~CompiledModule() {}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override
		{

		}

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override
		{

		}

		virtual bool validateModule() const override
		{
			return false;
		}

	};

}
