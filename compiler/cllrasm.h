
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

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
			
		public:
			Assembler(uint32_t initSize = 2048)
			{
				ssas.reserve(initSize);
			}

			virtual ~Assembler() {}

			SSA createSSA(Opcode op);

			SSA push(SSA ssa, Opcode op, std::array<uint32_t, 3> operands, bool genSSA = false);

			//void toSPIRV(spirv::Assembler* out);

			uint32_t addString(std::string str);

			std::string getString(uint32_t index)
			{
				return strs.at(index);
			}

		};

		class Emitter
		{
		public:
			virtual void getSSAs(cllr::Assembler& codeAsm) = 0;

			virtual void emitDeclCLLR(cllr::Assembler& codeAsm) = 0;

		};

	}

}