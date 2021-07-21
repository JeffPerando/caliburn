
#pragma once

#include <initializer_list>
#include <map>
#include <stdarg.h>
#include <string>
#include <vector>

#include "spirv.h"
#include "type.h"

namespace caliburn
{
	constexpr uint32_t CALIBURN_MAGIC_NUMBER = (0);

	struct SSA
	{
		uint32_t value = 0;
		SSA() {}
		SSA(uint32_t v) : value(v) {}
	};

	struct TypedSSA
	{
		uint32_t typeID;
		SSA value;
	};

	struct SpirVPhiOption
	{
		uint32_t label = 0;
		uint32_t ssa = 0;

		SpirVPhiOption() {}
		SpirVPhiOption(uint32_t l, uint32_t a) : label(l), ssa(a) {}
	};

	/*
	enum SSAType
	{
		DataType, LogicOp, Lvalue, Rvalue
	};
	*/
	
	enum Visibility
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

	Visibility strToVis(std::string str)
	{
		if (str == "public")
			return Visibility::PUBLIC;
		if (str == "protected")
			return Visibility::PROTECTED;
		if (str == "private")
			return Visibility::PRIVATE;
		if (str == "shared")
			return Visibility::SHARED;
		if (str == "static")
			return Visibility::STATIC;
		
		return NONE;
	}

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
		std::vector<SpvOp> ops = std::vector<SpvOp>(8192);
		std::map<std::string, SSA> typeAssigns;
		SpirVStack* currentStack = nullptr;
		uint32_t nextSSA = 0;
		int optimizeLvl = 0;

	public:
		SpirVAssembler(int o) : optimizeLvl(0) {}

		uint32_t newAssign()
		{
			nextSSA += 1;
			return nextSSA;
		}

		void push(uint32_t op);

		void pushAll(std::initializer_list<SpvOp> args);

		void pushStr(std::string str);

		uint32_t pushType(ParsedType* type);

		uint32_t pushSSA(SpvOp op);

		void startScope(uint32_t label);

		void endScope();

		void pushVarSetter(std::string name, uint32_t value);

		uint32_t getVar(std::string name);

		uint32_t* writeFile();

	};
}
