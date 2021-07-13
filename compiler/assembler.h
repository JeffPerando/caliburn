
#pragma once

#include <initializer_list>
#include <map>
#include <stdarg.h>
#include <string>
#include <vector>

#include "spirv.h"

namespace caliburn
{
	constexpr uint32_t CALIBURN_MAGIC_NUMBER = (0);

	struct SSA
	{
		uint32_t value = 0;
		SSA() {}
		SSA(uint32_t v) : value(v) {}
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

		void startScope(uint32_t label)
		{
			SpirVStack* scope = new SpirVStack();

			scope->label = label;

			if (currentStack)
			{
				currentStack->child = scope;
				scope->parent = currentStack;
				currentStack = scope;
				
			}
			else
			{
				currentStack = scope;

			}

		}

		void endScope()
		{
			SpirVStack* stack = currentStack;

			if (!stack)
			{
				//TODO complain
				return;
			}

			if (stack->child)
			{
				//TODO complain
				return;
			}

			stack->parent->child = nullptr;
			currentStack = stack->parent;

			delete stack;

			if (!currentStack)
			{
				return;
			}

			for (auto field : currentStack->fieldPhis)
			{
				SpirVPhi* phi = field.second;

				if (phi->options.size() < 2)
				{
					continue;
				}

				auto options = phi->options;

				auto fieldValue = newAssign();

				pushAll({ spirv::OpPhi(options.size() * 2), phi->type, fieldValue });

				for (auto option : phi->options)
				{
					pushAll({ option.label, option.ssa });

				}

				pushVarSetter(field.first, fieldValue);

			}

		}

		void pushVarSetter(std::string name, uint32_t value)
		{
			if (!currentStack)
			{
				//TODO complain
				return;
			}

			SpirVStack* stack = currentStack;
			SpirVStack* foundStack = nullptr;
			uint32_t bottomLabel = stack->label;

			while (stack)
			{
				uint32_t fieldSSA = stack->fields[name].value;

				if (fieldSSA)
				{
					foundStack = stack;
					break;
				}
				else
				{
					stack = stack->parent;

				}

			}

			if (!foundStack)
			{
				foundStack = currentStack;
			}

			foundStack->fields[name] = SSA(value);
			SpirVPhi* phi = foundStack->fieldPhis[name];

			if (!phi)
			{
				phi = new SpirVPhi();
				foundStack->fieldPhis[name] = phi;

			}

			phi->options.push_back(SpirVPhiOption(bottomLabel, value));

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
