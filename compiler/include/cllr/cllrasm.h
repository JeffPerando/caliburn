
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "ast/symbols.h"

#include "basic.h"
#include "cllr.h"
#include "langcore.h"

namespace caliburn
{
	namespace cllr
	{
		class Assembler
		{
		public:
			const ShaderType type;
		private:
			uint32_t nextSSA = 1;
			
			InstructionVec code;
			InstructionVec ssaToCode{ new_sptr<Instruction>() };
			std::vector<Opcode> ssaToOp{ Opcode::UNKNOWN };

			std::vector<std::string> strs;
			std::vector<uint32_t> ssaRefs{ 0 };
			
			//keep a stack of the current loop labels so we can implement break, continue, etc.
			std::vector<std::pair<SSA, SSA>> loops;

		public:
			Assembler(ShaderType t, uint32_t initSize = 2048) : type(t)
			{
				code.reserve(initSize);
				ssaToCode.reserve(initSize);
				ssaToOp.reserve(initSize);
				ssaRefs.reserve(initSize);

			}

			virtual ~Assembler() {}

			SSA createSSA(Opcode op);

			Opcode opFor(SSA id);

			sptr<Instruction> codeFor(SSA id);

			SSA push(SSA ssa, Opcode op, std::array<uint32_t, 3> operands, std::array<SSA, 3> refs, SSA type = 0, sptr<Token> debug = nullptr);

			SSA pushNew(Opcode op, std::array<uint32_t, 3> operands, std::array<SSA, 3> refs)
			{
				return push(createSSA(op), op, operands, refs);
			}

			uint32_t addString(std::string str);

			std::string getString(uint32_t index)
			{
				return strs.at(index);
			}
			
			uptr<InstructionVec> getCode()
			{
				return new_uptr<InstructionVec>(code);
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

			void findRefs(SSA id, ref<InstructionVec> result, size_t off = 0) const;

			void findPattern(ref<InstructionVec> result,
				Opcode opcode,
				std::array<bool, 3> ops, std::array<uint32_t, 3> opValues,
				std::array<bool, 3> ids, std::array<SSA, 3> idValues, size_t off = 0,
				size_t limit = UINT64_MAX) const;

			void findAll(ref<InstructionVec> result, const std::vector<Opcode> ops, size_t off = 0, size_t limit = UINT64_MAX) const;

			uint32_t replace(SSA in, SSA out);

			/*
			During optimization, certain instructions may end up becoming unused, and entire SSAs go unreferred. This
			method will remove those unused SSA indices and 'flatten' down the SSA list to eliminate bubbles. So say we
			have this list of 5 SSAs:

			SSA:	[1, 2, 3, 4, 5]
			Refs:	[6, 9, 0, 1, 3]
			
			flatten() will move #4 to #3, then move #5 to #4, and so on, updating surrounding code to match these changes.
			Running this on the example above, we get the following results:

			SSA:	[1, 2, 3, 4]
			Refs:	[6, 9, 1, 3]

			Returns the number of SSAs flattened.
			*/
			uint32_t flatten();

		};

		class Emitter
		{
		public:
			virtual cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;

		};

	}

}