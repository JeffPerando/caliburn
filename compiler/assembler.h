
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
		CaliburnAssembler()
		{
			//int types
			for (int bits = 8; bits <= 512; bits *= 2)
			{
				auto intType = new TypeInt(bits, true);
				auto uintType = new TypeInt(bits, false);

				defaultTypes.emplace("int" + bits, intType);
				defaultTypes.emplace("uint" + bits, uintType);

				defaultIntTypes.push_back(intType);
				defaultIntTypes.push_back(uintType);

			}

			//float types
			for (int bits = 16; bits <= 64; bits *= 2)
			{
				auto floatType = new TypeFloat(bits);
				defaultTypes.emplace("float" + bits, floatType);
				defaultFloatTypes.push_back(floatType);

			}

			//vector types

			CompiledType* fp16 = getFloatType(2);
			CompiledType* fp32 = getFloatType(4);
			CompiledType* fp64 = getFloatType(8);
			CompiledType* si32 = getIntType(4);

			for (int e = 2; e <= 4; ++e)
			{
				defaultTypes.emplace("hvec" + e, new TypeVector(e, fp16));
				defaultTypes.emplace("vec" + e, new TypeVector(e, fp32));
				defaultTypes.emplace("dvec" + e, new TypeVector(e, fp64));
				defaultTypes.emplace("ivec" + e, new TypeVector(e, si32));

			}

			for (size_t i = 2; i <= 4; ++i)
			{
				defaultVectorTypes.push_back((TypeVector*)defaultTypes["vec" + i]);
			}

			//void type
			defaultVoidType = new TypeVoid();
			defaultTypes.emplace("void", defaultVoidType);

			//typical aliases
			addPermanentAlias("byte", "int8");
			//TODO replace with proper character types that ensures chars get printed as chars
			addPermanentAlias("char", "uint8");
			addPermanentAlias("char8", "uint8");
			addPermanentAlias("char32", "uint32");
			addPermanentAlias("short", "int16");
			addPermanentAlias("int", "int32");
			addPermanentAlias("long", "int64");
			addPermanentAlias("float", "float32");
			addPermanentAlias("double", "float64");

		}

		virtual ~CaliburnAssembler()
		{
			for (auto type : defaultTypes)
			{
				delete type.second;

			}

		}

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
