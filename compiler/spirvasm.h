
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

        struct Type
        {
            SpvOp opcode = 0;
            SSA id = 0;
            std::vector<uint32_t> operands;

            bool operator<(const Type& rhs) const
            {
                if (opcode < rhs.opcode)
                    return true;
                /*
                //Don't compare the IDs
                //This struct will go in a map and we need to find pre-existing types
                if (id < rhs.id)
                    return true;
                */
                if (operands.size() < rhs.operands.size())
                    return true;

                if (operands.size() > rhs.operands.size())
                    return false;

                for (size_t i = 0; i < operands.size(); ++i)
                {
                    if (operands[i] < rhs.operands[i])
                        return true;

                }

                return false;
            }

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

            void push(SpvOp op, SSA id, std::vector<uint32_t> args);

            void pushVal(SpvOp op, SSA type, SSA id, std::vector<uint32_t> args);

            void pushRaw(std::vector<uint32_t> args);

            void pushStr(std::string str);

            SSA find(SpvOp op, std::vector<uint32_t> args);

        private:
            bool isValidOp(SpvOp op)
            {
                if (validOps.empty())
                {
                    return true;
                }

                if (std::binary_search(validOps.begin(), validOps.end(), op))
                {
                    return true;
                }

                return false;
            }

            void dump(ref<std::vector<uint32_t>> codeOut)
            {
                codeOut.insert(codeOut.end(), code.begin(), code.end());
            }

        };

	}

}
