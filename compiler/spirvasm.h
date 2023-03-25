
#pragma once

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "spirv.h"

namespace caliburn
{
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

        class Assembler;

        class CodeSection
        {
            Assembler* const codeAsm;
            std::vector<SpvOp> validOps;
            std::vector<uint32_t> code;
        public:
            CodeSection(Assembler* code, std::initializer_list<SpvOp> ops) : codeAsm(code), validOps(ops)
            {
                //OpNop is always valid
                if (!validOps.empty())
                {
                    validOps.push_back(spirv::OpNop());
                    std::sort(validOps.begin(), validOps.end());
                }

            }

            SSA push(SpvOp op, std::initializer_list<uint32_t> args, bool genSSA = true);

            void pushStr(std::string str);

        };

        class Assembler
        {
            uint32_t ssa = 1;
            std::vector<Capability> capabilities;
            std::vector<std::string> extensions, instructions;
            AddressingModel addrModel = AddressingModel::Logical;
            MemoryModel memModel = MemoryModel::GLSL450;
            std::vector<EntryPoint> entries;
            CodeSection spvImports, spvTypes, spvDecs, spvConsts, spvGloVars, spvCode;

            std::vector<SSAEntry> ssaEntries;

        public:
            Assembler() :
                spvImports(this, { OpExtInstImport() }),
                spvTypes(this, {
                    OpTypeArray(),
                    OpTypeBool(),
                    OpTypeFloat(),
                    OpTypeFunction(),
                    OpTypeImage(),
                    OpTypeInt(),
                    OpTypeMatrix(),
                    OpTypeOpaque(),
                    OpTypePointer(),
                    OpTypeSampler(),
                    OpTypeStruct(),
                    OpTypeVector(),
                    OpTypeVoid()
                        }),
                spvDecs(this, {
                    OpDecorate(),
                    OpGroupDecorate(),
                    OpGroupMemberDecorate(),
                    OpMemberDecorate(),
                    OpMemberDecorateString(),
                    OpDecorationGroup()
                        }),
                spvConsts(this, {
                    OpConstant(),
                    OpConstantComposite(),
                    OpConstantFalse(),
                    OpConstantNull(),
                    OpConstantSampler(),
                    OpConstantTrue(),
                    OpSpecConstantComposite(),
                    OpSpecConstantFalse(),
                    OpSpecConstantOp(),
                    OpSpecConstantTrue(),
                        }),
                spvGloVars(this, {
                    OpVariable()
                        }),
                spvCode(this, {})
            {}
            virtual ~Assembler() {}
            
            CodeSection* main()
            {
                return &spvCode;
            }

            CodeSection* types()
            {
                return &spvTypes;
            }

            CodeSection* consts()
            {
                return &spvConsts;
            }

            CodeSection* decors()
            {
                return &spvDecs;
            }

            SSA createSSA(SpvOp op);

            void addExt(std::string ext);

            //popular import is "GLSL.std.450"
            SSA addImport(SpvOp op, std::string instructions);

            SSA addGlobalVar(SSA type, StorageClass stClass, SSA init = 0);

            void addEntryPoint(SSA fn, ExecutionModel type, std::initializer_list<uint32_t> ios);

        };

	}
}
