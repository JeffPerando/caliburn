
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

        /*
        Enum for the different kinds of code section in SPIR-V
        */
        enum class SpvSection
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

        /*
        Represents a logical code section in SPIR-V.

        Uses the SPIR-V out assembler to generate SSA IDs, since the alternative is extremely error-prone.
        */
        struct CodeSection
        {
            friend struct cllr::SPIRVOutAssembler;

        private:
            const SpvSection section;
            //DO NOT DELETE
            //DO NOT TRY TO TURN INTO A SMART POINTER
            const ptr<cllr::SPIRVOutAssembler> spvAsm;

            //A list of all the ops that can be accepted in this section
            std::vector<SpvOp> validOps;
            //the actual code this section contains
            std::vector<uint32_t> code;
            
            //SSA to variable data
            std::map<SSA, VarData> varMeta;

            //Op length validation
            spirv::SpvOp lastOp = spirv::SpvOp();
            size_t lastOpOffset = 0;

            void pushOp(SpvOp op);

        public:
            CodeSection(SpvSection sec, ptr<cllr::SPIRVOutAssembler> spv, in<std::vector<SpvOp>> ops) : section(sec), spvAsm(spv), validOps(ops)
            {
                if (!validOps.empty())
                {
                    validOps.push_back(spirv::OpNop());
                    std::sort(validOps.begin(), validOps.end());
                }

            }

            /*
            Pushes a general operation, with optional ID

            If ID is 0, it will not be pushed
            */
            void push(SpvOp op, SSA id, in<std::vector<uint32_t>> args = {});

            /*
            Pushes a typed operation

            Will complain if type is 0
            */
            void pushTyped(SpvOp op, SSA type, SSA id, in<std::vector<uint32_t>> args);

            /*
            Pushes a variable
            */
            void pushVar(SSA type, SSA id, StorageClass sc, SSA init);

            /*
            Pushes raw data to the code
            */
            void pushRaw(in<std::vector<uint32_t>> args);

            /*
            Pushes a packed string directly into the code
            */
            void pushStr(std::string str);

            /*
            Pushes an OpDecorate with the given ID and metadata
            */
            void decorate(SSA target, Decoration d, in<std::vector<uint32_t>> args);

            /*
            Pushes an OpDecorateMember with the given ID and metadata
            */
            void decorateMember(SSA target, uint32_t member, Decoration d, in<std::vector<uint32_t>> args);

            /*
            Pushes a built-in decoration
            */
            void decorateBuiltIn(SSA target, BuiltIn b)
            {
                decorate(target, Decoration::BuiltIn, { (uint32_t)b });
            }

            /*
            Looks through the code for an op with the given arguments. Uses the length in op to determine how long
            */
            SSA find(SpvOp op, std::vector<uint32_t> args);

            /*
            Retrieves the variable data for the given ID.

            Returns true if the data was found, false otherwise.
            */
            bool getVarMeta(SSA id, out<VarData> meta);

        private:
            bool isValidOp(SpvOp op)
            {
                //indicates that only OpNop is inside
                if (validOps.empty())
                {
                    return true;
                }

                return std::binary_search(validOps.begin(), validOps.end(), op);
            }

            void dump(out<std::vector<uint32_t>> codeOut)
            {
                codeOut.insert(codeOut.end(), code.begin(), code.end());
            }

        };

	}

}
