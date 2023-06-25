
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
            CodeSection(ptr<cllr::SPIRVOutAssembler> spv, std::vector<SpvOp> ops) : spvAsm(spv), validOps(ops)
            {
                //OpNop is always valid
                if (!validOps.empty())
                {
                    validOps.push_back(OpNop());
                    std::sort(validOps.begin(), validOps.end());
                }

            }

            void push(SpvOp op, SSA id, std::vector<uint32_t> args = {});

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

        struct Type
        {
            SpvOp opcode = 0;
            SSA id = 0;
            std::vector<uint32_t> operands;

            bool operator!=(const Type& rhs) const
            {
                if (opcode != rhs.opcode)
                {
                    return true;
                }

                if (operands.size() != rhs.operands.size())
                    return true;

                for (size_t i = 0; i < operands.size(); ++i)
                {
                    if (operands[i] != rhs.operands[i])
                        return true;

                }

                return false;
            }

            bool operator==(const Type& rhs) const
            {
                return !(*this != rhs);
            }

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

        struct TypeHash
        {
            size_t operator() (const Type& t) const
            {
                size_t hash = t.opcode;

                //NO HASHING THE ID
                //hash ^= t.id;

                for (auto i : t.operands)
                {
                    hash << 18;//shift by an arbitrary amount to enable for more hashes, since we're only doing 32-bit ints
                    hash ^= i;
                }

                return hash;
            }
        };

        class TypeSection
        {
            const uptr<HashMap<spirv::Type, spirv::SSA, TypeHash>> types = new_uptr<HashMap<spirv::Type, spirv::SSA, TypeHash>>();

            const ptr<cllr::SPIRVOutAssembler> spvAsm;

        public:
            TypeSection(ptr<cllr::SPIRVOutAssembler> spv) : spvAsm(spv) {}

            SSA findOrMake(SpvOp op, std::vector<uint32_t> args = {}, SSA id = 0);

            void pushType(SpvOp op, SSA id, std::vector<uint32_t> args = {});

            void dump(ref<CodeSection> sec) const;

        };

        struct Constant
        {
            SSA type = 0;
            SSA id = 0;
            uint32_t lower = 0;
            uint32_t upper = 0;
            
            bool operator<(const Constant& rhs) const
            {
                if (type < rhs.type)
                    return true;
                /*
                //Don't compare the IDs
                //This struct will go in a map and we need to find pre-existing types
                if (id < rhs.id)
                    return true;
                */

                if (upper < rhs.upper)
                    return true;

                if (lower < rhs.lower)
                    return true;

                return false;
            }

        };

        class ConstSection
        {
            const uptr<std::map<Constant, spirv::SSA>> consts = new_uptr<std::map<Constant, spirv::SSA>>();

            const ptr<cllr::SPIRVOutAssembler> spvAsm;

        public:
            ConstSection(ptr<cllr::SPIRVOutAssembler> spv) : spvAsm(spv) {}

            SSA findOrMake(SSA t, float fp)
            {
                return findOrMake(t, *((uint32_t*)&fp));//hacky, I know
            }

            SSA findOrMake(SSA t, uint32_t first, uint32_t second = 0);

            void dump(ref<CodeSection> sec) const;

        };

	}

}
