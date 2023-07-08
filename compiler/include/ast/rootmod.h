
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
		HashMap<std::string, sptr<Module>> modules;
		
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
			//TODO resolve modules somehow

		}

		void declareSymbols(sptr<SymbolTable> table) override
		{
			for (auto const&[name, mod] : modules)
			{
				table->add(name, mod);

			}

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{

		}

		bool validateModule() const override
		{
			return false;
		}

	};

	struct CompiledModule : public Module
	{
		std::string name;

		HashMap<std::string, sptr<Variable>> vars;
		HashMap<std::string, sptr<BaseType>> types;
		HashMap<std::string, sptr<FunctionStatement>> fns;
		HashMap<std::string, sptr<ShaderStatement>> shaders;

		CompiledModule() {}
		virtual ~CompiledModule() {}

		void declareSymbols(sptr<SymbolTable> table) override
		{

		}

		void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{

		}

		bool validateModule() const override
		{
			return false;
		}

	};

}
