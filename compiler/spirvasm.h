
#pragma once

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "basic.h"
#include "spirv.h"

namespace caliburn
{
    namespace cllr
    {
        class SPIRVOutAssembler;
    }

    namespace spirv
    {
        struct SSAEntry
        {
            SSA ssa;
            uint32_t instruction;
        };

        struct EntryPoint
        {
            SSA func;
            ExecutionModel type;
            std::vector<uint32_t> io;
        };

        class CodeSection
        {
            friend class cllr::SPIRVOutAssembler;

            //DO NOT DELETE
            //DO NOT TRY TO TURN INTO A SMART POINTER
            const ptr<cllr::SPIRVOutAssembler> spvAsm;

            std::vector<SpvOp> validOps;
            std::vector<uint32_t> code;
        public:
            CodeSection(ptr<cllr::SPIRVOutAssembler> spv, std::initializer_list<SpvOp> ops) : spvAsm(spv), validOps(ops)
            {
                //OpNop is always valid
                if (!validOps.empty())
                {
                    validOps.push_back(OpNop());
                    std::sort(validOps.begin(), validOps.end());
                }

            }

            void push(SpvOp op, SSA id, std::initializer_list<uint32_t> args);

            void pushRaw(std::initializer_list<uint32_t> args);

            void pushRaw(std::vector<uint32_t> args);

            void pushStr(std::string str);

        private:
            void dump(ref<std::vector<uint32_t>> codeOut)
            {
                codeOut.insert(codeOut.end(), code.begin(), code.end());
            }

        };

	}

}
