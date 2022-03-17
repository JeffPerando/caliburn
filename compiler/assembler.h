
#pragma once

#include "deftypes.h"
#include "langcore.h"

namespace caliburn
{
	class CaliburnAssembler
	{
		SymbolTable stdLib;
		std::vector<CompiledType*> defaultTypes;

		std::vector<TypeInt*> defaultIntTypes;
		std::vector<TypeFloat*> defaultFloatTypes;
		std::vector<TypeVector*> defaultVectorTypes;
		TypeVoid* defaultVoidType = nullptr;

		void addDefaultType(std::string name, CompiledType* type)
		{
			if (type == nullptr)
			{
				return;
			}

			defaultTypes.push_back(type);
			
			auto sym = new TypeSymbol();

			sym->name = name;
			sym->type = type;

			stdLib.add(sym);

		}

	public:
		CaliburnAssembler();

		virtual ~CaliburnAssembler();

		//Scope helpers

		virtual void startScope() = 0;

		virtual void endScope() = 0;

		void addModule(SymbolTable* syms) {}


		void addTypeAlias(std::string alias, std::string original)
		{
			Symbol* sym = stdLib.resolve(original);

			if (!sym || sym->symbolType != SymbolType::TYPE)
			{
				return;
			}

			stdLib.alias(alias, sym);

		}

		TypeInt* getIntType(uint32_t bytes, bool sign = true)
		{
			for (auto it : defaultIntTypes)
			{
				if (it->getSizeBytes() == bytes && (it->hasA(TypeAttrib::SIGNED) == sign))
				{
					return it;
				}

			}

			return nullptr;
		}

		TypeFloat* getFloatType(uint32_t bytes)
		{
			for (auto ft : defaultFloatTypes)
			{
				if (ft->getSizeBytes() == bytes)
				{
					return ft;
				}

			}

			return nullptr;
		}

		TypeVector* getVecType(uint32_t elements)
		{
			if (elements > 4 || elements < 2)
			{
				return nullptr;
			}
			
			return defaultVectorTypes[elements - 2];
		}

		TypeVoid* getVoidType()
		{
			return defaultVoidType;
		}

		const std::vector<TypeInt*>* getAllIntTypes()
		{
			return &defaultIntTypes;
		}

		const std::vector<TypeFloat*>* getAllFloatTypes()
		{
			return &defaultFloatTypes;
		}

		const std::vector<TypeVector*>* getAllVecTypes()
		{
			return &defaultVectorTypes;
		}

	};

}
