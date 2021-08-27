
#pragma once

#include <cstdint>
#include <initializer_list>
#include <map>
#include <stdarg.h>
#include <string>
#include <vector>

#include "assembler.h"
#include "spirv.h"

namespace caliburn
{
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

	class SpirVAssembler : public CaliburnAssembler
	{
	private:
		std::vector<SpvOp> ops;
		std::vector<CompiledType*> usedTypes;

		std::map<uint64_t, TypedSSA> intConstants;

		SpirVStack* currentStack = nullptr;
		uint32_t nextSSA = 0;

	public:
		SpirVAssembler(size_t opCount = 8192)
		{
			ops = std::vector<SpvOp>(opCount);

		}

		//SSA helpers and pushers

		uint32_t newAssign()
		{
			nextSSA += 1;
			return nextSSA;
		}

		//Overrides

		virtual void startScope();

		virtual void endScope();

		//Scope helpers

		SpirVStack* getCurrentStack()
		{
			return currentStack;
		}

		//Assembly pushers

		void push(uint32_t op);

		void pushAll(std::initializer_list<SpvOp> args);

		void pushStr(std::string str);

		void pushVarSetter(std::string name, uint32_t value);

		uint32_t getVar(std::string name);

		//Type helpers

		TypedSSA getOrPushIntConst(uint64_t i, uint32_t bytes = 4, bool sign = true)
		{
			TypedSSA ret = intConstants[i];

			if (ret.value == 0)
			{
				ret.type = getIntType(bytes, sign);
				ret.value = newAssign();
				pushAll({ spirv::OpConstant(bytes / 4), ret.type->getSSA(), ret.value });

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