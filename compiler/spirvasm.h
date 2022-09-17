
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

            SSA push(SpvOp op, std::initializer_list<uint32_t> args, bool hasSSA = true);

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
            CodeSection imports, types, decorations, constants, globalVars, code;

        public:
            Assembler() :
                imports(this, { OpExtInstImport() }),
                types(this, {
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
                decorations(this, {
                    OpDecorate(),
                    OpGroupDecorate(),
                    OpGroupMemberDecorate(),
                    OpMemberDecorate(),
                    OpMemberDecorateString(),
                    OpDecorationGroup()
                        }),
                constants(this, {
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
                globalVars(this, {
                    OpVariable()
                        }),
                code(this, {}) {}
            virtual ~Assembler() {}

            CodeSection* main() { return &code; }

            CodeSection* types() { return &types; }

            CodeSection* consts() { return &constants; }

            CodeSection* decors() { return &decorations; }

            SSA nextSSA(SpvOp op);

            void pushExt(std::string ext);

            //popular import is "GLSL.std.450"
            SSA pushImport(SpvOp op, std::string instructions);

            SSA pushGlobalVar(SSA type, StorageClass stClass, SSA init = 0);

        };

	}
}
