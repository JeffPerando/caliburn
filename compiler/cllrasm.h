
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "basic.h"
#include "langcore.h"
#include "cllr.h"

namespace caliburn
{
	namespace cllr
	{
		class Assembler
		{
		public:
			const ShaderType type;
		private:
			uint32_t nextSSA = 0;
			
			std::vector<sptr<Instruction>> code;

			std::vector<cllr::Opcode> ssaToOpcode;
			//std::vector<sptr<Instruction>> ssaToCode;
			std::vector<std::string> strs;
			std::vector<uint32_t> ssaRefs;
			
			//keep a stack of the current loop labels so we can implement break, continue, etc.
			std::vector<std::pair<SSA, SSA>> loops;

		public:
			Assembler(ShaderType t, uint32_t initSize = 2048) : type(t)
			{
				code.reserve(initSize);
				ssaRefs.reserve(initSize);
				ssaToOpcode.reserve(initSize);
				
				ssaRefs.push_back(0);

				ssaToOpcode.push_back(Opcode::UNKNOWN);//ID 0 is unused

			}

			virtual ~Assembler() {}

			SSA createSSA(Opcode op);

			SSA push(SSA ssa, Opcode op, std::array<uint32_t, 3> operands, std::array<SSA, 3> refs);

			SSA pushNew(Opcode op, std::array<uint32_t, 3> operands, std::array<SSA, 3> refs)
			{
				return push(createSSA(op), op, operands, refs);
			}

			uint32_t addString(std::string str);

			std::string getString(uint32_t index)
			{
				return strs.at(index);
			}
			
			uptr<std::vector<sptr<Instruction>>> getCode()
			{
				return std::make_unique<std::vector<sptr<Instruction>>>(code);
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

			void findRefs(SSA id, ref<std::vector<sptr<Instruction>>> result);

			void findPattern(ref<std::vector<sptr<Instruction>>> result,
				Opcode opcode,
				std::array<bool, 3> ops, std::array<uint32_t, 3> opValues,
				std::array<bool, 3> ids, std::array<SSA, 3> idValues,
				size_t limit = UINT64_MAX);

			uint32_t replace(SSA in, SSA out);

		};

		class Emitter
		{
		public:
			virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) = 0;

		};

	}

}