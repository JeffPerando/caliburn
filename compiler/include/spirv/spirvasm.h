
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
        struct SPIRVOutAssembler;
    }

    namespace spirv
    {
        struct SSAEntry
        {
            SSA ssa = 0;
            SpvOp instruction = spirv::OpNop();
        };

        struct EntryPoint
        {
            SSA func;
            ExecutionModel type;
            std::string name;
            std::vector<uint32_t> io;
        };

        struct VarData
        {
            SSA id;
            SSA innerType;
            StorageClass strClass;
        };

        enum class SSAKind
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

        struct CodeSection
        {
            friend struct cllr::SPIRVOutAssembler;

        private:
            const SSAKind section;
            //DO NOT DELETE
            //DO NOT TRY TO TURN INTO A SMART POINTER
            const ptr<cllr::SPIRVOutAssembler> spvAsm;

            std::vector<SpvOp> validOps;
            std::vector<uint32_t> code;
            
            std::map<SSA, VarData> varMeta;

        public:
            CodeSection(SSAKind sec, ptr<cllr::SPIRVOutAssembler> spv, std::vector<SpvOp> ops) : section(sec), spvAsm(spv), validOps(ops)
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

            void decorate(SSA target, Decoration d, std::vector<uint32_t> args);

            void decorateMember(SSA target, uint32_t member, Decoration d, std::vector<uint32_t> args);

            void decorateBuiltIn(SSA target, BuiltIn b)
            {
                decorate(target, Decoration::BuiltIn, { (uint32_t)b });
            }

            SSA find(SpvOp op, std::vector<uint32_t> args);

            bool findVarMeta(SSA id, ref<VarData> meta);

        private:
            bool isValidOp(SpvOp op)
            {
                //OpNop is always valid
                if (op == OpNop())
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

	}

}
