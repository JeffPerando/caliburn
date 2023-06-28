
#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "basic.h"

#include "spirv/spirv.h"

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

        struct VarData
        {
            SSA id;
            SSA innerType;
            StorageClass strClass;
        };

        enum class SSASection
        {
            INVALID,
            UNKNOWN,
            HEADER,
            IMPORT,
            HEADER2,
            DEBUG,
            TYPE,
            CONST,
            DECORATION,
            GLOBAL_VAR,
            MAIN
        };

        class CodeSection
        {
            friend class cllr::SPIRVOutAssembler;

            //DO NOT DELETE
            //DO NOT TRY TO TURN INTO A SMART POINTER
            const SSASection section;
            const ptr<cllr::SPIRVOutAssembler> spvAsm;

            std::vector<SpvOp> validOps;
            std::vector<uint32_t> code;
            
            std::map<SSA, VarData> varMeta;

        public:
            CodeSection(SSASection sec, ptr<cllr::SPIRVOutAssembler> spv, std::vector<SpvOp> ops) : section(sec), spvAsm(spv), validOps(ops)
            {
                if (!validOps.empty())
                {
                    std::sort(validOps.begin(), validOps.end());
                }

            }

            void push(SpvOp op, SSA id, std::vector<uint32_t> args = {});

            void pushTyped(SpvOp op, SSA type, SSA id, std::vector<uint32_t> args);

            void pushVar(SSA type, SSA id, StorageClass sc, SSA init);

            void pushRaw(std::vector<uint32_t> args);

            void pushStr(std::string str);

            SSA find(SpvOp op, std::vector<uint32_t> args);

            bool findVarMeta(SSA id, ref<VarData> meta);

        private:
            bool isValidOp(SpvOp op)
            {
                //OpNop is always valid
                if (op == spirv::OpNop().op)
                {
                    return true;
                }

                if (validOps.empty())
                {
                    return true;
                }

                return std::binary_search(validOps.begin(), validOps.end(), op);
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
                    hash <<= 18;//shift by an arbitrary amount to enable for more hashes, since we're only doing 32-bit ints
                    hash ^= i;
                }

                return hash;
            }
        };

        class TypeSection
        {
            const uptr<HashMap<spirv::Type, spirv::SSA, TypeHash>> types = new_uptr<HashMap<spirv::Type, spirv::SSA, TypeHash>>();
            const uptr<HashMap<spirv::SSA, spirv::Type>> ssaToType = new_uptr<HashMap<spirv::SSA, spirv::Type>>();

            const ptr<cllr::SPIRVOutAssembler> spvAsm;

        public:
            TypeSection(ptr<cllr::SPIRVOutAssembler> spv) : spvAsm(spv) {}

            bool findData(SSA id, ref<Type> out);

            SSA findOrMake(SpvOp op, std::vector<uint32_t> args = {}, SSA id = 0);

            void pushNew(SpvOp op, SSA id, std::vector<uint32_t> args = {});

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

        struct CompositeConst
        {
            SSA typeID = 0;
            SSA id = 0;
            std::vector<uint32_t> data;

            bool operator<(const CompositeConst& rhs) const
            {
                if (typeID < rhs.typeID)
                    return true;
                /*
                //Don't compare the IDs
                //This struct will go in a map and we need to find pre-existing types
                if (id < rhs.id)
                    return true;
                */

                if (data.size() < rhs.data.size())
                    return true;

                for (size_t i = 0; i < rhs.data.size(); ++i)
                {
                    if (data[i] < rhs.data[i])
                        return true;
                }

                return false;
            }

        };

        class ConstSection
        {
            const uptr<std::map<Constant, spirv::SSA>> consts = new_uptr<std::map<Constant, spirv::SSA>>();
            const uptr<std::map<CompositeConst, spirv::SSA>> composites = new_uptr<std::map<CompositeConst, spirv::SSA>>();
            const uptr<HashMap<spirv::SSA, spirv::SSA>> nulls = new_uptr<HashMap<spirv::SSA, spirv::SSA>>();

            const ptr<cllr::SPIRVOutAssembler> spvAsm;

        public:
            ConstSection(ptr<cllr::SPIRVOutAssembler> spv) : spvAsm(spv) {}

            SSA findOrMakeFP32(SSA t, float fp)
            {
                return findOrMake(t, *((uint32_t*)&fp));//hacky, I know
            }

            SSA findOrMakeFP64(SSA t, double dfp)
            {
                auto raw = *((uint64_t*)&dfp);

                uint32_t first = (raw & 0xFFFFFFFF);
                uint32_t second = ((raw >> 32) & 0xFFFFFFFF);

                return findOrMake(t, first, second);
            }

            SSA findOrMake(SSA t, uint32_t first, uint32_t second = 0);

            SSA findOrMakeComposite(SSA t, std::vector<uint32_t> data);

            SSA findOrMakeNullFor(SSA t);

            void dump(ref<CodeSection> sec) const;

        };

	}

}
