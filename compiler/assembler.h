
#pragma once

#include <initializer_list>
#include <map>
#include <stdarg.h>
#include <string>
#include <vector>

#include "deftypes.h"
#include "spirv.h"

namespace caliburn
{
	class Statement;

	constexpr uint32_t CALIBURN_MAGIC_NUMBER = (0);

	struct SSA
	{
		uint32_t value = 0;
		SSA() {}
		SSA(uint32_t v) : value(v) {}

		operator uint32_t()
		{
			return value;
		}
	};

	struct TypedSSA
	{
		CompiledType* type = nullptr;
		SSA value = 0;
	};

	struct SpirVPhiOption
	{
		uint32_t label = 0;
		uint32_t ssa = 0;

		SpirVPhiOption() {}
		SpirVPhiOption(uint32_t l, uint32_t a) : label(l), ssa(a) {}
	};

	enum class Visibility
	{
		NONE,
		PUBLIC,
		PROTECTED,
		PRIVATE,
		//Shared is here because it's semantically in the same location;
		//i.e. "int shared x;" vs. "int public x;"
		//Shared is only valid as a local field; All others are valid everywhere else.
		SHARED,
		STATIC

	};

	Visibility strToVis(std::string str);

	struct SpirVPhi
	{
		std::string fieldName;
		uint32_t type = 0;
		std::vector<SpirVPhiOption> options;

	};

	struct SpirVStack
	{
		SpirVStack* parent = nullptr;
		uint32_t label = 0;
		std::map<std::string, SSA> fields;
		std::map<std::string, SpirVPhi*> fieldPhis;
		SpirVStack* child = nullptr;

		~SpirVStack()
		{
			for (auto phi : fieldPhis)
			{
				delete phi.second;
			}
		}

	};

	class SpirVAssembler
	{
	private:
		std::vector<SpvOp> ops;
		std::vector<CompiledType*> usedTypes;
		
		std::map<std::string, CompiledType*> defaultTypes;
		
		std::map<std::pair<uint32_t, bool>, TypeInt*> defaultIntTypes;
		TypeFloat* defaultFloatTypes[9]{ nullptr };
		std::map<uint64_t, TypedSSA> intConstants;

		SpirVStack* currentStack = nullptr;
		uint32_t nextSSA = 0;
		
	public:
		SpirVAssembler(size_t opCount = 8192)
		{
			ops = std::vector<SpvOp>(opCount);

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
			//TODO replace with proper character type that modifies the behavior of the type
			addPermanentAlias("char", "uint8");
			addPermanentAlias("short", "int16");
			addPermanentAlias("int", "int32");
			addPermanentAlias("long", "int64");

			addPermanentAlias("float", "float32");
			addPermanentAlias("double", "float64");

		}

		~SpirVAssembler()
		{
			for (auto type : defaultTypes)
			{
				delete type.second;

			}

		}

		//SSA helpers and pushers
		
		uint32_t newAssign()
		{
			nextSSA += 1;
			return nextSSA;
		}

		void push(uint32_t op);

		void pushAll(std::initializer_list<SpvOp> args);

		void pushStr(std::string str);

		void pushVarSetter(std::string name, uint32_t value);

		uint32_t getVar(std::string name);

		//Scope helpers

		void startScope(uint32_t label);

		void endScope();

		//Type helpers

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

		TypedSSA getOrPushIntConst(uint64_t i, uint32_t bytes = 4, bool sign = true)
		{
			TypedSSA ret = intConstants[i];

			if (ret.value == 0)
			{
				ret.type = getIntType(bytes, sign);
				ret.value = newAssign();
				pushAll({spirv::OpConstant(bytes / 4), ret.type->getSSA(), ret.value});
				
				if (bytes > 4)
				{
					push(uint32_t(i >> 32) & 0xFFFFFFFF);
				}

				push(uint32_t(i) & 0xFFFFFFFF);

			}

			return ret;
		}

	};

}
