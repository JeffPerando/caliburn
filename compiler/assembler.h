
#pragma once

#include "deftypes.h"

namespace caliburn
{
	class CaliburnAssembler
	{
		std::map<std::string, CompiledType*> defaultTypes;

		std::map<std::pair<uint32_t, bool>, TypeInt*> defaultIntTypes;
		TypeFloat* defaultFloatTypes[9]{ nullptr };

	public:
		CaliburnAssembler()
		{
			//int types
			for (int bits = 8; bits <= 512; bits *= 2)
			{
				for (int s = 0; s < 2; ++s)
				{
					bool hasSign = (s == 1);
					auto intType = new TypeInt(bits, hasSign);
					defaultTypes.emplace((s ? "int" : "uint") + bits, intType);
					defaultIntTypes.emplace(std::pair<uint32_t, bool>(bits / 8, hasSign), intType);

				}

			}

			//float types
			for (int bits = 16; bits <= 64; bits *= 2)
			{
				auto floatType = new TypeFloat(bits);
				defaultTypes.emplace("float" + bits, floatType);
				defaultFloatTypes[bits / 8] = floatType;

			}

			//vector types

			CompiledType* fp16 = getFloatType(2);
			CompiledType* fp32 = getFloatType(4);
			CompiledType* fp64 = getFloatType(8);
			CompiledType* si32 = getIntType(4);

			for (int e = 2; e <= 4; ++e)
			{
				defaultTypes.emplace("hvec" + e, new TypeVector("hvec" + e, e, fp16));
				defaultTypes.emplace("vec" + e, new TypeVector("vec" + e, e, fp32));
				defaultTypes.emplace("dvec" + e, new TypeVector("dvec" + e, e, fp64));
				defaultTypes.emplace("ivec" + e, new TypeVector("ivec" + e, e, si32));

			}

			//void type
			defaultTypes.emplace("void", new TypeVoid());

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

		CompiledType* resolveType(ParsedType* type);

		void addPermanentAlias(std::string alias, std::string original)
		{
			defaultTypes[alias] = defaultTypes.at(original);
		}

		TypeInt* getIntType(uint32_t bytes, bool sign = true)
		{
			return defaultIntTypes[std::pair<uint32_t, bool>(bytes, sign)];
		}

		TypeFloat* getFloatType(uint32_t bytes)
		{
			return defaultFloatTypes[bytes];
		}

	};

}
