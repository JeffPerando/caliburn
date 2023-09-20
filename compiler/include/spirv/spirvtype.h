
#pragma once

#include <vector>

#include "spirvasm.h"

#include "basic.h"

namespace caliburn
{
    namespace cllr
    {
        struct SPIRVOutAssembler;
    }

	namespace spirv
	{
        struct Type
        {
            SpvOp opcode = 0;
            SSA id = 0;
            std::vector<uint32_t> operands;

            bool operator!=(in<Type> rhs) const
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

            bool operator==(in<Type> rhs) const
            {
                return !(*this != rhs);
            }

            bool operator<(in<Type> rhs) const
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
            size_t operator() (in<Type> t) const
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

        /*
        Maintains the comprehensive list of types within SPIR-V

        Also includes a fair few helper methods
        */
        class TypeSection
        {
            const uptr<HashMap<Type, SSA, TypeHash>> types = new_uptr<HashMap<Type, SSA, TypeHash>>();
            const uptr<HashMap<SSA, Type>> ssaToType = new_uptr<HashMap<SSA, Type>>();

            const ptr<cllr::SPIRVOutAssembler> spvAsm;

        public:
            TypeSection(ptr<cllr::SPIRVOutAssembler> spv) : spvAsm(spv) {}

            bool findData(SSA id, out<Type> out);

            SSA findOrMake(SpvOp op, in<std::vector<uint32_t>> args = {}, SSA id = 0);

            //void pushNew(SpvOp op, SSA id, std::vector<uint32_t> args = {});

            void dump(out<CodeSection> sec) const;

            SSA typeInt(uint32_t width = 32);

            SSA typeUInt(uint32_t width = 32);

            SSA typeFP(uint32_t width = 32);

            SSA typeBool();

            SSA typeVec(uint32_t len, SSA inner = 0);

            SSA typeArray(uint32_t len, SSA inner);

            SSA typeStruct(in<std::vector<uint32_t>> members, in<std::vector<BuiltIn>> decs = {});

            SSA typePtr(SSA inner, StorageClass sc);

            SSA typeInPtr(SSA inner)
            {
                return typePtr(inner, StorageClass::Input);
            }

            SSA typeOutPtr(SSA inner)
            {
                return typePtr(inner, StorageClass::Output);
            }

        };

	}

}