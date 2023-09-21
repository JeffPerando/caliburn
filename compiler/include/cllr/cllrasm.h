
#pragma once

#include <map>
#include <stack>
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
		/*
		A LowType is a concrete, CLLR-level type. It has no generics, instead being one of Caliburn's fundamental base types.
		*/
		struct LowType;

		/*
		Used to maintain a list of current inputs and outputs. Mainly used to make VertexInputAttribute structs for the front-end API
		*/
		using IOList = std::vector<std::pair<std::string, uint32_t>>;

		/*
		The CLLR Assembler is a tool to help generate CLLR code. It keeps track of errors, the current SSA index,
		the generated code itself, lookups to help speed up algorithms, inputs, outputs, constant strings, loops,
		helper methods...

		Yes, all of this is necessary for generating CLLR code.
		*/
		struct Assembler
		{
		public:
			const ShaderType type;
			const uptr<ErrorHandler> errors;

			sptr<const CompilerSettings> settings;

		private:
			const uptr<InstructionVec> code = new_uptr<InstructionVec>();

			uint32_t nextSSA = 1;

			std::vector<size_t> ssaToIndex{ 0 };
			std::vector<Opcode> ssaToOp{ Opcode::UNKNOWN };
			std::vector<uint32_t> ssaRefs{ 0 };

			HashMap<Instruction, std::pair<SSA, sptr<LowType>>, InstructionHash> types;

			std::vector<std::string> strs;
			
			std::map<std::string, std::pair<uint32_t, SSA>> inputs, outputs;
			
			//Makes outputting a shader's 'API' easy.
			IOList inputNames, outputNames;

			//keep a stack of the current loop labels so we can implement break, continue, etc.
			std::stack<std::pair<SSA, SSA>> loops;
		public:
			Assembler(ShaderType t, sptr<const CompilerSettings> cs, uint32_t initSize = 2048) :
				type(t), settings(cs), errors(new_uptr<ErrorHandler>(CompileStage::CLLR_EMIT, cs))
			{
				code->reserve(initSize);
				ssaToIndex.reserve(initSize);
				ssaToOp.reserve(initSize);
				ssaRefs.reserve(initSize);

			}

			virtual ~Assembler() {}

			SSA createSSA(Opcode op);

			out<Instruction> codeFor(SSA id) const;

			out<Instruction> codeAt(size_t off) const;

			Opcode opFor(SSA id) const;

			SSA push(in<Instruction> ins);

			SSA pushNew(out<Instruction> ins)
			{
				ins.index = createSSA(ins.op);
				return push(ins);
			}

			SSA pushType(in<Instruction> ins);

			void pushAll(in<std::vector<Instruction>> code);

			sptr<LowType> getType(SSA id) const;

			std::pair<uint32_t, SSA> pushInput(in<std::string>, SSA type);

			std::pair<uint32_t, SSA> pushOutput(in<std::string> name, SSA type);

			uint32_t addString(in<std::string> str);

			std::string getString(uint32_t index) const
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
				loops.push(std::pair(start, end));
			}

			SSA getLoopStart() const
			{
				if (loops.empty())
				{
					return 0;
				}

				return loops.top().first;
			}

			SSA getLoopEnd() const
			{
				if (loops.empty())
				{
					return 0;
				}

				return loops.top().second;
			}

			void exitLoop()
			{
				if (!loops.empty())
				{
					loops.pop();
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
			void doBookkeeping(in<Instruction> i);

		};

		/*
		Contract for any struct which emits valid CLLR code.

		Values use a different method due to having to include their type.
		*/
		struct Emitter
		{
			virtual cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) = 0;

		};

	}

}