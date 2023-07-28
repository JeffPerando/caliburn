
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "basic.h"
#include "cllr.h"
#include "cllrtypeimpl.h"
#include "error.h"
#include "langcore.h"

#include "ast/symbols.h"

namespace caliburn
{
	namespace cllr
	{
		struct Assembler
		{
		public:
			const ShaderType type;
		private:
			uint32_t nextSSA = 1;
			
			const uptr<InstructionVec> code = new_uptr<InstructionVec>();

			InstructionVec ssaToCode{ new_sptr<Instruction>() };
			std::vector<Opcode> ssaToOp{ Opcode::UNKNOWN };
			std::vector<uint32_t> ssaRefs{ 0 };

			HashMap<Instruction, std::pair<SSA, sptr<LowType>>, InstructionHash> types;

			std::vector<std::string> strs;
			
			std::set<std::string> ioNames;
			std::map<std::string, SSA> inputs, outputs;

			//keep a stack of the current loop labels so we can implement break, continue, etc.
			std::vector<std::pair<SSA, SSA>> loops;

		public:
			const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::CLLR_EMIT);

			Assembler(ShaderType t, uint32_t initSize = 2048) : type(t)
			{
				code->reserve(initSize);
				ssaToCode.reserve(initSize);
				ssaToOp.reserve(initSize);
				ssaRefs.reserve(initSize);

			}

			virtual ~Assembler() {}

			SSA createSSA(Opcode op);

			sptr<Instruction> codeFor(SSA id) const;

			SSA push(ref<Instruction> ins);

			SSA pushNew(ref<Instruction> ins)
			{
				ins.index = createSSA(ins.op);
				return push(ins);
			}

			SSA pushType(ref<Instruction> ins);

			void pushAll(std::vector<Instruction> code);

			sptr<LowType> getType(SSA id) const;

			std::pair<SSA, uint32_t> pushInput(std::string name, SSA type);

			std::pair<SSA, uint32_t> pushOutput(std::string name, SSA type);

			uint32_t addString(std::string str);

			ref<const std::string> getString(uint32_t index) const
			{
				return strs.at(index);
			}
			
			const ref<const InstructionVec> getCode() const
			{
				return *code;
			}
			
			void setLoop(SSA start, SSA end)
			{
				loops.push_back(std::pair(start, end));
			}

			SSA getLoopStart() const
			{
				if (loops.empty())
				{
					return 0;
				}

				return loops.back().first;
			}

			SSA getLoopEnd() const
			{
				if (loops.empty())
				{
					return 0;
				}

				return loops.back().second;
			}

			void exitLoop()
			{
				if (!loops.empty())
				{
					loops.pop_back();
				}

			}

			void addIOName(std::string name);

			/*
			Replaces all references to 'in' with 'out'. This includes output types.

			This is not an aliasing method; This will change instructions.

			*/
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
			
		private:
			void doBookkeeping(sptr<Instruction> i);

		};

		class Emitter
		{
		public:
			virtual cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;

		};

	}

}