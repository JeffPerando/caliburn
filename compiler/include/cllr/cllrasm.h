
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
			const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::CLLR_EMIT);

			sptr<const CompilerSettings> settings;

		private:
			uint32_t nextSSA = 1;
			uint32_t nextInput = 0;
			uint32_t nextOutput = 0;

			std::vector<Instruction> allCode;
			std::stack<uptr<Section>> codeSects;

			HashMap<SSA, Instruction> ssaToIns{ {0, Instruction()} };
			std::vector<uint32_t> ssaRefs{ 0 };

			HashMap<Instruction, sptr<LowType>, InstructionHash> types;
			HashMap<SSA, sptr<LowType>> ssaToType;

			std::vector<std::string> strs;

			std::map<std::string_view, IOVar> ioVars;
			std::map<std::string_view, TypedSSA> ioVarIDs;

		public:
			Assembler(ShaderType t, sptr<const CompilerSettings> cs, std::vector<IOVar> vars) :
				type(t), settings(cs)
			{
				for (auto const& var : vars)
				{
					ioVars.emplace(var.name, var);
				}

			}

			virtual ~Assembler() = default;

			const std::vector<Instruction>& getCode() const
			{
				return allCode;
			}

			SSA beginSect(out<Instruction> i);
			bool hasSect() const;
			Instruction getSectHeader();
			void endSect(in<Instruction> i);

			SSA createSSA(in<Instruction> ins);

			Instruction getIns(SSA id) const;
			Opcode getOp(SSA id) const;
			sptr<LowType> getType(SSA id) const;
			
			void push(in<Instruction> ins);
			void pushAll(in<std::vector<Instruction>> code);
			SSA pushNew(out<Instruction> ins);
			TypedSSA pushValue(out<Instruction> ins, sptr<LowType> type);
			sptr<LowType> pushType(Opcode op);
			sptr<LowType> pushType(out<Instruction> ins);
			TypedSSA pushIOVar(std::string_view name, ShaderIOVarType type, sptr<LowType> dataType);

			void beginLoop(SSA start, SSA end);
			SSA getLoopStart() const;
			SSA getLoopEnd() const;
			void endLoop();

			size_t addString(in<std::string> str);
			std::string getString(size_t index) const;

			std::vector<IOVar> getIOByType(ShaderIOVarType type) const
			{
				std::vector<IOVar> inputs;

				for (auto const& [name, io] : ioVars)
				{
					if (io.type == type)
					{
						inputs.push_back(io);
					}
				}

				return inputs;
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
			const Instruction header;
			Section(in<Instruction> i) : header(i)
			{
				code.push_back(i);
			}
			virtual ~Section() {}

			void beginLoop(SSA start, SSA end);
			SSA getLoopStart() const;
			SSA getLoopEnd() const;
			void endLoop();

		};

	}

}