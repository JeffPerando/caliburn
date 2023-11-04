
#pragma once

#include <map>
#include <stack>
#include <string>
#include <vector>

#include "basic.h"
#include "error.h"
#include "langcore.h"

#include "cllr.h"

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

		struct Section;

		/*
		The CLLR Assembler is a tool to help generate CLLR code. It keeps track of errors, the current SSA index,
		the generated code itself, lookups to help speed up algorithms, inputs, outputs, constant strings, loops,
		helper methods...

		Yes, all of this is necessary for generating CLLR code.
		*/
		struct Assembler
		{
			const ShaderType type;
			const uptr<ErrorHandler> errors;

			sptr<const CompilerSettings> settings;

		private:
			std::vector<Instruction> allCode;
			std::stack<uptr<Section>> codeSects;

			uint32_t nextSSA = 1;

			std::vector<Instruction> ssaToIns;
			std::vector<Opcode> ssaToOp;
			std::vector<uint32_t> ssaRefs{ 0 };

			HashMap<Instruction, sptr<LowType>, InstructionHash> types;
			HashMap<SSA, sptr<LowType>> ssaToType;

			std::vector<std::string> strs;

			std::map<std::string, std::pair<uint32_t, SSA>> inputs, outputs;

			//Makes outputting a shader's 'API' easy.
			IOList inputNames, outputNames;

		public:
			Assembler(ShaderType t, sptr<const CompilerSettings> cs) :
				type(t), settings(cs), errors(new_uptr<ErrorHandler>(CompileStage::CLLR_EMIT, cs)) {}
			virtual ~Assembler() {}

			ref<const std::vector<Instruction>> getCode() const
			{
				return allCode;
			}

			void beginSect();
			bool hasSect() const;
			void endSect();

			SSA createSSA(in<Instruction> ins);

			Instruction getIns(SSA id) const;
			Opcode getOp(SSA id) const;
			sptr<LowType> getType(SSA id) const;

			void push(in<Instruction> ins);
			void pushAll(in<std::vector<Instruction>> code);
			SSA pushNew(out<Instruction> ins);
			sptr<LowType> pushType(out<Instruction> ins);
			
			void beginLoop(SSA start, SSA end);
			SSA getLoopStart() const;
			SSA getLoopEnd() const;
			void endLoop();

			uint32_t addString(in<std::string> str);
			std::string getString(uint32_t index) const;

			std::pair<uint32_t, SSA> pushInput(in<std::string>, SSA type);
			std::pair<uint32_t, SSA> pushOutput(in<std::string> name, SSA type);

			ref<const IOList> getInputs() const
			{
				return inputNames;
			}

			ref<const IOList> getOutputs() const
			{
				return outputNames;
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

		struct Section
		{
			friend struct Assembler;

		private:
			std::vector<Instruction> code;
			//keep a stack of the current loop labels so we can implement break, continue, etc.
			std::stack<Loop> loops;

		public:
			Section() {}
			virtual ~Section() {}
			
			void beginLoop(SSA start, SSA end);
			SSA getLoopStart() const;
			SSA getLoopEnd() const;
			void endLoop();

		};

	}

}