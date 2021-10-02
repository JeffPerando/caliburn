
#pragma once

#include "deftypes.h"

namespace caliburn
{
	class CaliburnAssembler
	{
		std::map<std::string, CompiledType*> defaultTypes;

		std::vector<TypeInt*> defaultIntTypes;
		std::vector<TypeFloat*> defaultFloatTypes;
		std::vector<TypeVector*> defaultVectorTypes;
		TypeVoid* defaultVoidType = nullptr;

	public:
		CaliburnAssembler();

		virtual ~CaliburnAssembler();

		//Scope helpers

		virtual void startScope() = 0;

		virtual void endScope() = 0;

		CompiledType* resolveType(std::string name)
		{
			return resolveType("", name);
		}

		CompiledType* resolveType(std::string mod, std::string name)
		{
			ParsedType type;
			Token modTkn(mod);
			Token nameTkn(name);

			type.mod = &modTkn;
			type.name = &nameTkn;
			
			return resolveType(&type);
		}

		CompiledType* resolveType(ParsedType* type);

		void addPermanentAlias(std::string alias, std::string original)
		{
			defaultTypes[alias] = defaultTypes.at(original);
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
