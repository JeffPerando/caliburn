
#pragma once

#include <initializer_list>
#include <map>
#include <stdarg.h>
#include <string>
#include <vector>

#include "spirv.h"

#define CALIBURN_MAGIC_NUMBER (0)

namespace caliburn
{
	struct SSA
	{
		uint32_t value = 0;
		SSA() : value(0) {}
		SSA(uint32_t v) : value(v) {}
	};
	/*
	enum SSAType
	{
		DataType, LogicOp, Lvalue, Rvalue
	};
	*/
	enum Operator
	{
		//==, >, <
		EQUALS, GREATER, LESSER,
		//+, -, *, /
		ADD, SUB, MUL, DIV,
		//%, ^, &, |
		MOD, POW, BIT_AND, BIT_OR,
		//$, ++, //
		BIT_XOR, APPEND, INTDIV,
		//!, ~, -
		BOOL_NOT, BIT_NOT, NEGATE,
		//|x|, x[n]
		ABS, ARRAY_ACCESS
	};

	enum Visibility
	{
		PUBLIC,
		PROTECTED,
		PRIVATE,
		//Shared is here because it's semantically in the same location;
		//i.e. "int shared x;" vs. "int public x;"
		//Shared is only valid as a local field; All others are valid everywhere else.
		SHARED,
		STATIC
	};

	struct SpirVStack
	{
		SpirVStack* parent = nullptr;
		uint32_t label = 0;
		std::map<std::string, SSA> fields;
		SpirVStack* child = nullptr;
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

		uint32_t pushType(std::string name)
		{
			uint32_t ssa = typeAssigns[name].value;

			if (!ssa)
			{
				ssa = newAssign();
				typeAssigns[name] = SSA(ssa);
				
			}

			return ssa;
		}

		uint32_t pushSSA(SpvOp op)
		{
			auto ssa = newAssign();

			ops.push_back(op);
			ops.push_back(ssa);

			return ssa;
		}

		void push(uint32_t op)
		{
			ops.push_back(op);

		}

		void pushAll(std::initializer_list<SpvOp> args)
		{
			ops.insert(ops.end(), args);
			
		}

		void pushStr(std::string str)
		{
			size_t copied = (((str.length() & ~0x3) >> 2) + ((str.length() & 0x3) > 0 * 1) + 1);
			ops.resize(ops.size() + copied, 0);
			std::memcpy(ops.end()._Ptr - copied, str.c_str(), str.length());

		}

		void pushVarSetter(std::string name, uint32_t value)
		{
			if (!currentStack)
			{
				//TODO complain
			}

			SpirVStack* stack = currentStack;

			while (stack)
			{
				uint32_t fieldSSA = stack->fields[name].value;

				if (fieldSSA)
				{
					stack->fields[name] = SSA(value);
					//TODO add phi instruction helper
				}
				else
				{
					stack = stack->parent;

				}

			}

			//TODO complain

		}

		uint32_t getVar(std::string name)
		{
			if (!currentStack)
			{
				//TODO complain
			}

			SpirVStack* stack = currentStack;

			while (stack)
			{
				uint32_t fieldSSA = stack->fields[name].value;

				if (fieldSSA)
				{
					return fieldSSA;
				}

				stack = stack->parent;

			}

			return 0;
		}

		uint32_t* writeFile()
		{
			ops.push_back(spirv::MagicNumber());
			ops.push_back(spirv::Version(1, 0));
			ops.push_back(CALIBURN_MAGIC_NUMBER);
			ops.push_back(0);//REMEMBER TO SET THIS TO THE BOUNDS
			ops.push_back(0);

		}

	};
}
