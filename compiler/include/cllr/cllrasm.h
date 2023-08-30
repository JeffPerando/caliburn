
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "basic.h"
#include "cllr.h"
#include "error.h"
#include "langcore.h"

#include "ast/symbols.h"

namespace caliburn
{
	namespace cllr
	{
		struct LowType;

		using IOList = std::vector<std::pair<std::string, uint32_t>>;

		struct Assembler
		{
		public:
			const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::CLLR_EMIT);

			uint32_t nextSSA = 1;

			const ShaderType type;
			sptr<const CompilerSettings> settings;
			
		private:
			const uptr<InstructionVec> code = new_uptr<InstructionVec>();

			std::vector<size_t> ssaToIndex{ 0 };
			std::vector<Opcode> ssaToOp{ Opcode::UNKNOWN };
			std::vector<uint32_t> ssaRefs{ 0 };

			HashMap<Instruction, std::pair<SSA, sptr<LowType>>, InstructionHash> types;

			std::vector<std::string> strs;
			
			std::map<std::string, std::pair<uint32_t, SSA>> inputs, outputs;
			
			//Makes outputting a shader's 'API' easy.
			IOList inputNames, outputNames;

			//keep a stack of the current loop labels so we can implement break, continue, etc.
			std::vector<std::pair<SSA, SSA>> loops;
		public:
			Assembler(ShaderType t, sptr<const CompilerSettings> cs, uint32_t initSize = 2048) : type(t), settings(cs)
			{
				code->reserve(initSize);
				ssaToIndex.reserve(initSize);
				ssaToOp.reserve(initSize);
				ssaRefs.reserve(initSize);

			}

			virtual ~Assembler() {}

			SSA createSSA(Opcode op);

			ref<const Instruction> codeFor(SSA id) const;

			ref<const Instruction> codeAt(size_t off) const;

			Opcode opFor(SSA id) const;

			SSA push(ref<Instruction> ins);

			SSA pushNew(ref<Instruction> ins)
			{
				ins.index = createSSA(ins.op);
				return push(ins);
			}

			SSA pushType(ref<Instruction> ins);

			void pushAll(std::vector<Instruction> code);

			sptr<LowType> getType(SSA id) const;

			std::pair<uint32_t, SSA> pushInput(std::string name, SSA type);

			std::pair<uint32_t, SSA> pushOutput(std::string name, SSA type);

			uint32_t addString(ref<const std::string> str);

			ref<const std::string> getString(uint32_t index) const
			{
				if (strs.empty() || index >= strs.size())
				{
					return "";
				}

				return strs.at(index);
			}
			
			ref<const IOList> getInputs()
			{
				return inputNames;
			}

			ref<const IOList> getOutputs()
			{
				return outputNames;
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
			void doBookkeeping(ref<Instruction> i);

		};

		class Emitter
		{
		public:
			virtual cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) = 0;

		};

	}

}