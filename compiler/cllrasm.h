
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "basic.h"
#include "cllr.h"

namespace caliburn
{
	namespace cllr
	{
		class Assembler
		{
			uint32_t nextSSA = 0;
			std::vector<uint32_t> ssas;
			std::vector<std::string> strs;
			std::vector<Instruction> code;
			std::map<uint32_t, uint32_t> ssaAliases;
			
			//keep a stack of the current loop labels so we can implement break, continue, etc.
			std::vector<std::pair<SSA, SSA>> loops;

		public:
			Assembler(uint32_t initSize = 2048)
			{
				ssas.reserve(initSize);
			}

			virtual ~Assembler() {}

			SSA createSSA(Opcode op);

			SSA push(SSA ssa, Opcode op, std::array<uint32_t, 3> operands, std::array<SSA, 3> refs, bool genSSA = false);

			//void toSPIRV(spirv::Assembler* out);

			uint32_t addString(std::string str);

			std::string getString(uint32_t index)
			{
				return strs.at(index);
			}

			std::vector<Instruction>* getCode()
			{
				return &code;
			}

			void setLoop(SSA start, SSA end)
			{
				loops.push_back(std::pair(start, end));
			}

			SSA getLoopStart()
			{
				return loops.back().first;
			}

			SSA getLoopEnd()
			{
				return loops.back().second;
			}

			void exitLoop()
			{
				if (!loops.empty())
				{
					loops.pop_back();
				}

			}

		};

		class Emitter
		{
		public:
			virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) = 0;

		};

	}

}