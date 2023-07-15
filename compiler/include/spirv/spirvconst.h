
#pragma once

#include "spirvasm.h"

namespace caliburn
{
    namespace cllr
    {
        struct SPIRVOutAssembler;
    }

	namespace spirv
	{
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
            const uptr<std::map<Constant, SSA>> consts = new_uptr<std::map<Constant, SSA>>();
            const uptr<std::map<CompositeConst, SSA>> composites = new_uptr<std::map<CompositeConst, SSA>>();
            const uptr<HashMap<SSA, SSA>> nulls = new_uptr<HashMap<SSA, SSA>>();

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

                uint32_t low = (raw & 0xFFFFFFFF);
                uint32_t high = ((raw >> 32) & 0xFFFFFFFF);

                return findOrMake(t, low, high);
            }

            SSA findOrMake(SSA t, uint32_t low, uint32_t high = 0);

            SSA findOrMakeComposite(SSA t, std::vector<uint32_t> data);

            SSA findOrMakeNullFor(SSA t);

            void dump(ref<CodeSection> sec) const;

        };

	}

}