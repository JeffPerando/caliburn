
#pragma once

#include <stdint.h>

#define SPIRV_Op(Name, ID, Count) SpvOp inline Name() {return SpvOp(Count, ID);}
#define SPIRV_OpVar(Name, ID, Base) SpvOp inline Name(uint32_t var = 0) {return SpvOp(Base + var, ID);}

namespace caliburn
{
    namespace spirv
    {
        constexpr uint32_t SPIRV_FILE_MAGIC_NUMBER = 0x07230203;
        //1975 is the year Monty Python and the Holy Grail came out
        constexpr uint32_t CALIBURN_GENERATOR_MAGIC_NUMBER = 1975;

        using SSA = uint32_t;

        struct SpvOp
        {
            uint16_t words = 0;
            uint16_t op = 0;

            SpvOp() {}

            SpvOp(uint32_t code)
            {
                words = ((code >> 16) & 0xFFFF);
                op = (code & 0xFFFF);
            }

            SpvOp(uint16_t wordCount, uint16_t opcode) : words(wordCount), op(opcode) {}

            operator uint32_t()
            {
                return *(uint32_t*)this;
            }

            bool operator<(const SpvOp& op) const
            {
                return this->op < op.op;
            }

        };

        struct Version
        {
        private:
            const uint8_t unused_0 = 0;
        public:
            uint8_t major;
            uint8_t minor;
        private:
            const uint8_t unused_1 = 0;

            Version(uint8_t maj = 1, uint8_t min = 0) : major(maj), minor(min) {}

            operator uint32_t()
            {
                return *(uint32_t*)this;
            }

        };

        struct FuncControl
        {
            uint32_t Inline : 1,
                DontInline : 1,
                Pure : 1,
                Const : 1;

            FuncControl()
            {
                Inline = 0;
                DontInline = 0;
                Pure = 0;
                Const = 0;
            }

            operator uint32_t()
            {
                return *(uint32_t*)this;
            }

        };

        enum class Capability : uint32_t
        {
            Matrix =            0,
            Shader =            1,
            Geometry =          2,
            Tessellation =      3,
            Addresses =         4,
            Linkage =           5,
            Kernel =            6,
            Vector16 =          7,
            Float16Buffer =     8,
            Float16 =           9,
            Float64 =           10,
            Int64 =             11,
            Int64Atomics =      12,
            ImageBasic =        13,
            ImageReadWrite =    14,
            ImageMipmap =       15,
            Pipes =             17,
            Groups =            18,
            DeviceEnqueue =     19,
            LiteralSampler =    20,
            AtomicStorage =     21,
            Int16 =             22,
            TessellationPointSize = 23,
            GeometryPointSize = 24,
            ImageGatherExtended = 25,
            //number 26 is missing???
            StorageImageMultisample = 27,
            UniformBufferArrayDynamicIndexing = 28,
            SampledImageArrayDynamicIndexing = 29,
            StorageBufferArrayDynamicIndexing = 30,
            StorageImageArrayDynamicIndexing = 31,
            ClipDistance = 32,
            CullDistance = 33,
            ImageCubeArray = 34,
            SampleRateShading = 35,
            ImageRect = 36,
            SampledRect = 37,
            GenericPointer = 38,
            Int8 = 39,
            InputAttachment = 40,
            SparseResidency = 41,
            MinLod = 42,
            Sampled1D = 43,
            Image1D = 44,
            SampledCubeArray = 45,
            SampledBuffer = 46,
            ImageBuffer = 47,
            ImageMSArray = 48
            //TODO add more
        };

        enum class AddressingModel : uint32_t
        {
            Logical =                   0,
            Physical32 =                1,
            Physical64 =                2,
            PhysicalStorageBuffer64 =   5348
        };

        enum class MemoryModel : uint32_t
        {
            Simple = 0,
            GLSL450 = 1,
            OpenCL = 2,
            Vulkan = 3,
            VulkanKHR = 3
        };

        //Not complete, missing a lot of RT extension enums
        enum class StorageClass : uint32_t
        {
            UniformConstant = 0,
            Input = 1,
            Uniform = 2,
            Output = 3,
            Workgroup = 4,
            CrossWorkgroup = 5,
            Private = 6,
            Function = 7,
            Generic = 8,
            PushConstant = 9,
            AtomicCounter = 10,
            Image = 11,
            //version >= 1.3
            StorageBuffer = 12,
            CallableDataKHR = 5328,
            IncomingCallableDataKHR = 5329,
            RayPayloadKHR = 5338,
            HitAttributeKHR = 5339,
            IncomingRayPayloadKHR = 5342,
            ShaderRecordBufferKHR = 5343,
            //version >= 1.5
            PhysicalStorageBuffer = 5349,
            PhysicalStorageBufferEXT = 5349
        };

        enum class ExecutionModel : uint32_t
        {
            Vertex = 0,
            TessellationControl = 1,
            TessellationEvaluation = 2,
            Geometry = 3,
            Fragment = 4,
            GLCompute = 5,
            Kernel = 6,
            RayGenerationKHR = 5313,
            IntersectionKHR = 5314,
            AnyHitKHR = 5315,
            ClosestHitKHR = 5316,
            MissKHR = 5317,
            CallableKHR = 5318
        };

        enum class Dim : uint32_t
        {
            _1D = 0,
            _2D = 1,
            _3D = 2,
            Cube = 3,
            Rect = 4,
            Buffer = 5,
            SubpassData = 6
        };

        enum class SamplerAddressingMode : uint32_t
        {
            None = 0,
            ClampToEdge = 1,
            Clamp = 2,
            Repeat = 3,
            RepeatMirrored = 4
        };

        struct ImageOperand
        {
            uint32_t Bias : 1,
                Lod : 1,
                Grad: 1,
                ConstOffset: 1,
                Offset: 1,
                ConstOffsets: 1,
                Sample: 1,
                MinLod: 1,
                MakeTexelAvailable: 1,
                MakeTexelVisible: 1,
                NonPrivateTexel: 1,
                VolatileTexel: 1,
                SignExtend: 1,
                ZeroExtend: 1;

            operator uint32_t()
            {
                return *(uint32_t*)this;
            }

        };

        //ONLY INSTRUCTIONS BELOW THIS POINT
        //also they're sorted by opcode
        //sorry

        SPIRV_Op(OpNop, 0, 1);

        SPIRV_Op(OpUndef, 1, 3);

        SPIRV_OpVar(OpSourceContinued, 2, 2);

        SPIRV_OpVar(OpSource, 3, 3);

        SPIRV_OpVar(OpSourceExtension, 4, 2);

        SPIRV_OpVar(OpName, 5, 3);

        SPIRV_OpVar(OpMemberName, 6, 4);

        SPIRV_OpVar(OpString, 7, 3);

        SPIRV_Op(OpLine, 8, 4);

        SPIRV_OpVar(OpExtension, 10, 2);

        SPIRV_OpVar(OpExtInstImport, 11, 3);

        SPIRV_OpVar(OpExtInst, 12, 5);

        SPIRV_Op(OpMemoryModel, 14, 3);

        SPIRV_OpVar(OpEntryPoint, 15, 4);

        SPIRV_OpVar(OpExecutionMode, 16, 3);

        SPIRV_Op(OpCapability, 17, 2);

        SPIRV_Op(OpTypeVoid, 19, 2);

        SPIRV_Op(OpTypeBool, 20, 2);

        SPIRV_Op(OpTypeInt, 21, 4);

        SPIRV_Op(OpTypeFloat, 22, 3);

        SPIRV_Op(OpTypeVector, 23, 4);

        SPIRV_Op(OpTypeMatrix, 24, 4);

        SPIRV_OpVar(OpTypeImage, 25, 9);

        SPIRV_Op(OpTypeSampler, 26, 2);

        SPIRV_Op(OpTypeSampledImage, 27, 3);

        SPIRV_Op(OpTypeArray, 28, 4);

        SPIRV_Op(OpTypeRuntimeArray, 29, 3);

        SPIRV_OpVar(OpTypeStruct, 30, 2);

        SPIRV_OpVar(OpTypeOpaque, 31, 3);

        SPIRV_Op(OpTypePointer, 32, 4);

        SPIRV_OpVar(OpTypeFunction, 33, 3);

        SPIRV_Op(OpTypeDevent, 34, 2);

        SPIRV_Op(OpTypeDeviceEvent, 35, 2);

        SPIRV_Op(OpTypeReserveId, 36, 2);

        SPIRV_Op(OpTypeQueue, 37, 2);

        SPIRV_Op(OpTypePipe, 38, 2);

        SPIRV_Op(OpTypeForwardPointer, 39, 3);

        SPIRV_Op(OpConstantTrue, 41, 3);

        SPIRV_Op(OpConstantFalse, 42, 3);

        SPIRV_OpVar(OpConstant, 43, 4);

        SPIRV_OpVar(OpConstantComposite, 44, 3);

        SPIRV_Op(OpConstantSampler, 45, 6);

        SPIRV_Op(OpConstantNull, 46, 3);

        SPIRV_Op(OpSpecConstantTrue, 48, 3);

        SPIRV_Op(OpSpecConstantFalse, 49, 3);

        SPIRV_OpVar(OpSpecConstant, 50, 4);

        SPIRV_OpVar(OpSpecConstantComposite, 51, 5);

        SPIRV_OpVar(OpSpecConstantOp, 52, 4);

        SPIRV_Op(OpFunction, 54, 5);

        SPIRV_Op(OpFunctionParameter, 55, 3);

        SPIRV_Op(OpFunctionEnd, 56, 1);

        SPIRV_OpVar(OpFunctionCall, 57, 4);

        SPIRV_OpVar(OpVariable, 59, 4);

        SPIRV_Op(OpImageTexelPointer, 60, 6);

        SPIRV_OpVar(OpLoad, 61, 4);

        SPIRV_OpVar(OpStore, 62, 3);

        SPIRV_OpVar(OpCopyMemory, 63, 3);

        SPIRV_OpVar(OpCopyMemorySized, 64, 4);

        SPIRV_OpVar(OpAccessChain, 65, 4);

        //Execute opcode 66
        SPIRV_OpVar(OpInBoundsAccessChain, 66, 4);

        SPIRV_OpVar(OpPtrAccessChain, 67, 5);

        SPIRV_Op(OpArrayLength, 68, 5);

        //nice
        SPIRV_Op(OpGenericPtrMemSemantics, 69, 4);

        SPIRV_OpVar(OpInBoundsPtrAccessChain, 70, 5);

        SPIRV_OpVar(OpDecorate, 71, 3);

        SPIRV_OpVar(OpMemberDecorate, 72, 4);

        SPIRV_Op(OpDecorationGroup, 73, 2);

        SPIRV_OpVar(OpGroupDecorate, 74, 2);

        SPIRV_OpVar(OpGroupMemberDecorate, 75, 2);

        SPIRV_Op(OpVectorExtractDynamic, 77, 5);

        SPIRV_Op(OpVectorInsertDynamic, 78, 6);

        SPIRV_OpVar(OpVectorShuffle, 79, 5);

        SPIRV_OpVar(OpCompositeConstruct, 80, 3);

        SPIRV_OpVar(OpCompositeExtract, 81, 4);

        SPIRV_OpVar(OpCompositeInsert, 82, 5);

        SPIRV_Op(OpCopyObject, 83, 4);

        SPIRV_Op(OpTranspose, 84, 4);

        SPIRV_Op(OpSampledImage, 86, 5);

        SPIRV_OpVar(OpImageSampleImplicitLod, 87, 5);

        SPIRV_OpVar(OpImageSampleExplicitLod, 88, 7);

        SPIRV_OpVar(OpImageSampleDrefImplicitLod, 89, 6);

        SPIRV_OpVar(OpImageSampleDrefExplicitLod, 90, 8);

        SPIRV_OpVar(OpImageSampleProjImplicitLod, 91, 5);

        SPIRV_OpVar(OpImageSampleProjExplicitLod, 92, 7);

        SPIRV_OpVar(OpImageSampleProjDrefImplicitLod, 93, 6);

        SPIRV_OpVar(OpImageSampleProjDrefExplicitLod, 94, 8);

        SPIRV_OpVar(OpImageFetch, 95, 5);

        SPIRV_OpVar(OpImageGather, 96, 6);

        SPIRV_OpVar(OpImageDrefGather, 97, 6);

        SPIRV_OpVar(OpImageRead, 98, 5);

        SPIRV_OpVar(OpImageWrite, 99, 4);

        SPIRV_Op(OpImage, 100, 4);

        SPIRV_Op(OpImageQueryFormat, 101, 4);

        SPIRV_Op(OpImageQueryOrder, 102, 4);

        SPIRV_Op(OpImageQuerySizeLod, 103, 5);

        SPIRV_Op(OpImageQuerySize, 104, 4);

        SPIRV_Op(OpImageQueryLod, 105, 5);

        SPIRV_Op(OpImageQueryLevels, 106, 4);

        SPIRV_Op(OpImageQuerySamples, 107, 4);

        SPIRV_Op(OpConvertFToU, 109, 4);

        SPIRV_Op(OpConvertFToS, 110, 4);

        SPIRV_Op(OpConvertSToF, 111, 4);

        SPIRV_Op(OpConvertUToF, 112, 4);

        SPIRV_Op(OpUConvert, 113, 4);

        SPIRV_Op(OpSConvert, 114, 4);

        SPIRV_Op(OpFConvert, 115, 4);

        SPIRV_Op(OpQuantizeToF16, 116, 4);

        SPIRV_Op(OpConvertPtrToU, 117, 4);

        SPIRV_Op(OpSatConvertSToU, 118, 4);

        SPIRV_Op(OpSatConvertUToS, 119, 4);

        SPIRV_Op(OpConvertUToPtr, 120, 4);

        SPIRV_Op(OpPtrCastToGeneric, 121, 4);

        SPIRV_Op(OpGenericCastToPtr, 122, 4);

        SPIRV_Op(OpGenericCastToPtrExplicit, 123, 5);

        SPIRV_Op(OpBitcast, 124, 4);

        SPIRV_Op(OpSNegate, 126, 4);

        SPIRV_Op(OpFNegate, 127, 4);

        SPIRV_Op(OpIAdd, 128, 5);

        SPIRV_Op(OpFAdd, 129, 5);

        SPIRV_Op(OpISub, 130, 5);

        SPIRV_Op(OpFSub, 131, 5);

        SPIRV_Op(OpIMul, 132, 5);

        SPIRV_Op(OpFMul, 133, 5);

        SPIRV_Op(OpUDiv, 134, 5);

        SPIRV_Op(OpSDiv, 135, 5);

        SPIRV_Op(OpFDiv, 136, 5);

        SPIRV_Op(OpUMod, 137, 5);

        SPIRV_Op(OpSRem, 138, 5);

        SPIRV_Op(OpSMod, 139, 5);

        SPIRV_Op(OpFRem, 140, 5);

        SPIRV_Op(OpFMod, 141, 5);

        SPIRV_Op(OpVectorTimesScalar, 142, 5);

        SPIRV_Op(OpMatrixTimesScalar, 143, 5);

        SPIRV_Op(OpVectorTimesMatrix, 144, 5);

        SPIRV_Op(OpMatrixTimesVector, 145, 5);

        SPIRV_Op(OpMatrixTimesMatrix, 146, 5);

        SPIRV_Op(OpOuterProduct, 147, 5);

        SPIRV_Op(OpDot, 148, 5);

        SPIRV_Op(OpIAddCarry, 149, 5);

        SPIRV_Op(OpISubBorrow, 150, 5);

        SPIRV_Op(OpUMulExtended, 151, 5);

        SPIRV_Op(OpSMulExtended, 152, 5);

        SPIRV_Op(OpAny, 154, 4);

        SPIRV_Op(OpAll, 155, 4);

        SPIRV_Op(OpIsNan, 156, 4);

        SPIRV_Op(OpIsInf, 157, 4);

        SPIRV_Op(OpIsFinite, 158, 4);

        SPIRV_Op(OpIsNormal, 159, 4);

        SPIRV_Op(OpSignBitSet, 160, 4);

        //Deprecated
        SPIRV_Op(OpLessOrGreater, 161, 5);

        SPIRV_Op(OpOrdered, 162, 5);

        SPIRV_Op(OpUnordered, 163, 5);

        SPIRV_Op(OpLogicalEqual, 164, 5);

        SPIRV_Op(OpLogicalNotEqual, 165, 5);

        SPIRV_Op(OpLogicalOr, 166, 5);

        SPIRV_Op(OpLogicalAnd, 167, 5);

        SPIRV_Op(OpLogicalNot, 168, 4);

        SPIRV_Op(OpSelect, 169, 6);

        SPIRV_Op(OpIEqual, 170, 5);

        SPIRV_Op(OpINotEqual, 171, 5);

        SPIRV_Op(OpUGreaterThan, 172, 5);

        SPIRV_Op(OpSGreaterThan, 173, 5);

        SPIRV_Op(OpUGreaterThanEqual, 174, 5);

        SPIRV_Op(OpSGreaterThanEqual, 175, 5);

        SPIRV_Op(OpULessThan, 176, 5);

        SPIRV_Op(OpSLessThan, 177, 5);

        SPIRV_Op(OpULessThanEqual, 178, 5);

        SPIRV_Op(OpSLessThanEqual, 179, 5);

        SPIRV_Op(OpFOrdEqual, 180, 5);

        SPIRV_Op(OpFUnordEqual, 181, 5);

        SPIRV_Op(OpFOrdNotEqual, 182, 5);

        SPIRV_Op(OpFUnordNotEqual, 183, 5);

        SPIRV_Op(OpFOrdLessThan, 184, 5);

        SPIRV_Op(OpFUnordLessThan, 185, 5);

        SPIRV_Op(OpFOrdGreaterThan, 186, 5);

        SPIRV_Op(OpFUnordGreaterThan, 187, 5);

        SPIRV_Op(OpFOrdLessThanEqual, 188, 5);

        SPIRV_Op(OpFUnordLessThanEqual, 189, 5);

        SPIRV_Op(OpFOrdGreaterThanEqual, 190, 5);

        SPIRV_Op(OpFUnordGreaterThanEqual, 191, 5);

        SPIRV_Op(OpShiftRightLogical, 194, 5);

        SPIRV_Op(OpShiftRightArithmetic, 195, 5);

        SPIRV_Op(OpShiftLeftLogical, 196, 5);

        SPIRV_Op(OpBitwiseOr, 197, 5);

        SPIRV_Op(OpBitwiseXor, 198, 5);

        SPIRV_Op(OpBitwiseAnd, 199, 5);

        SPIRV_Op(OpNot, 200, 4);

        SPIRV_Op(OpBitFieldInsert, 201, 7);

        SPIRV_Op(OpBitFieldSExtract, 202, 6);

        SPIRV_Op(OpBitFieldUExtract, 203, 6);

        SPIRV_Op(OpBitReverse, 204, 4);

        SPIRV_Op(OpBitCount, 205, 4);

        SPIRV_Op(OpDPdx, 207, 4);

        SPIRV_Op(OpDPdy, 208, 4);

        SPIRV_Op(OpFwidth, 209, 4);

        SPIRV_Op(OpDPdxFine, 210, 4);

        SPIRV_Op(OpDPdyFine, 211, 4);

        SPIRV_Op(OpFwidthFine, 212, 4);

        SPIRV_Op(OpDPdxCoarse, 213, 4);

        SPIRV_Op(OpDPdyCoarse, 214, 4);

        SPIRV_Op(OpFwidthCoarse, 215, 4);

        SPIRV_Op(OpEmitVertex, 218, 1);

        SPIRV_Op(OpEndPrimitive, 219, 1);

        SPIRV_Op(OpEmitStreamVertex, 220, 1);

        SPIRV_Op(OpEndStreamPrimitive, 221, 2);

        SPIRV_Op(OpControlBarrier, 224, 4);

        SPIRV_Op(OpMemoryBarrier, 225, 4);

        SPIRV_Op(OpAtomicLoad, 227, 6);

        SPIRV_Op(OpAtomicStore, 228, 5);

        SPIRV_Op(OpAtomicExchange, 229, 7);

        SPIRV_Op(OpAtomicCompareExchange, 230, 9);

        SPIRV_Op(OpAtomicCompareExchangeWeak, 231, 9);

        SPIRV_Op(OpAtomicIIncrement, 232, 6);

        SPIRV_Op(OpAtomicIDecrement, 233, 6);

        SPIRV_Op(OpAtomicIAdd, 234, 7);

        SPIRV_Op(OpAtomicISub, 235, 7);

        SPIRV_Op(OpAtomicSMin, 236, 7);

        SPIRV_Op(OpAtomicUMin, 237, 7);

        SPIRV_Op(OpAtomicSMax, 238, 7);

        SPIRV_Op(OpAtomicUMax, 239, 7);

        SPIRV_Op(OpAtomicAnd, 240, 7);

        SPIRV_Op(OpAtomicOr, 241, 7);

        SPIRV_Op(OpAtomicXor, 242, 7);

        SPIRV_OpVar(OpPhi, 245, 3);

        SPIRV_OpVar(OpLoopMerge, 246, 4);

        SPIRV_Op(OpSelectionMerge, 247, 3);

        SPIRV_Op(OpLabel, 248, 2);

        SPIRV_Op(OpBranch, 249, 2);

        SPIRV_OpVar(OpBranchConditional, 250, 4);

        SPIRV_OpVar(OpSwitch, 251, 3);

        SPIRV_Op(OpKill, 252, 1);

        SPIRV_Op(OpReturn, 253, 1);

        SPIRV_Op(OpReturnValue, 254, 2);

        SPIRV_Op(OpUnreachable, 255, 1);

        SPIRV_Op(OpLifetimeStart, 256, 3);

        SPIRV_Op(OpLifetimeStop, 257, 3);

        SPIRV_Op(OpGroupAsyncCopy, 259, 9);

        SPIRV_Op(OpGroupWaitEvents, 260, 4);

        SPIRV_Op(OpGroupAll, 261, 5);

        SPIRV_Op(OpGroupAny, 262, 5);

        SPIRV_Op(OpGroupBroadcast, 263, 6);

        SPIRV_Op(OpGroupIAdd, 264, 6);

        SPIRV_Op(OpGroupFAdd, 265, 6);

        SPIRV_Op(OpGroupFMin, 266, 6);

        SPIRV_Op(OpGroupUMin, 267, 6);

        SPIRV_Op(OpGroupSMin, 268, 6);

        SPIRV_Op(OpGroupFMax, 269, 6);

        SPIRV_Op(OpGroupUMax, 270, 6);

        SPIRV_Op(OpGroupSMax, 271, 6);

        SPIRV_Op(OpReadPipe, 274, 7);

        SPIRV_Op(OpWritePipe, 275, 7);

        SPIRV_Op(OpReservedReadPipe, 276, 9);

        SPIRV_Op(OpReservedWritePipe, 277, 9);

        SPIRV_Op(OpReserveReadPipePackets, 278, 7);

        SPIRV_Op(OpReserveWritePipePackets, 279, 7);

        SPIRV_Op(OpCommitReadPipe, 280, 5);

        SPIRV_Op(OpCommitWritePipe, 281, 5);

        SPIRV_Op(OpIsValidReserveId, 282, 4);

        SPIRV_Op(OpGetNumPipePackets, 283, 5);

        SPIRV_Op(OpGetMaxPipePackets, 284, 6);

        SPIRV_Op(OpGroupReserveReadPipePackets, 285, 8);

        SPIRV_Op(OpGroupReserveWritePipePackets, 286, 8);

        SPIRV_Op(OpGroupCommitReadPipe, 287, 6);

        SPIRV_Op(OpGroupCommitWritePipe, 288, 6);

        SPIRV_Op(OpEnqueueMarker, 291, 7);

        SPIRV_OpVar(OpEnqueueKernel, 292, 13);

        SPIRV_Op(OpGetKernelNDrangeSubGroupCount, 293, 8);

        SPIRV_Op(OpGetKernelNDrangeMaxSubGroupSize, 294, 8);

        SPIRV_Op(OpGetKernelWorkGroupSize, 295, 7);

        SPIRV_Op(OpGetKernelPreferredWorkGroupSizeMultiple, 296, 7);

        SPIRV_Op(OpRetainEvent, 297, 2);

        SPIRV_Op(OpReleaseEvent, 298, 2);

        SPIRV_Op(OpCreateUserEvent, 299, 3);

        SPIRV_Op(OpIsValidEvent, 300, 4);

        SPIRV_Op(OpSetUserEventStatus, 301, 3);

        SPIRV_Op(OpCaptureEventProfilingInfo, 302, 4);

        SPIRV_Op(OpGetDefaultQueue, 303, 3);

        SPIRV_Op(OpBuildNDRange, 304, 6);

        SPIRV_OpVar(OpImageSparseSampleImplicitLod, 305, 5);

        SPIRV_OpVar(OpImageSparseSampleExplicitLod, 306, 7);

        SPIRV_OpVar(OpImageSparseSampleDrefImplicitLod, 307, 6);

        SPIRV_OpVar(OpImageSparseSampleDrefExplicitLod, 308, 8);

        SPIRV_OpVar(OpImageSparseSampleProjImplicitLod, 309, 5);

        SPIRV_OpVar(OpImageSparseSampleProjExplicitLod, 310, 7);

        SPIRV_OpVar(OpImageSparseSampleProjDrefImplicitLod, 311, 6);

        SPIRV_OpVar(OpImageSparseFetch, 313, 5);

        SPIRV_OpVar(OpImageSparseGather, 314, 6);

        SPIRV_OpVar(OpImageSparseDrefGather, 315, 6);

        SPIRV_Op(OpImageSparseTexelsResident, 316, 4);

        SPIRV_Op(OpNoLine, 317, 1);

        SPIRV_Op(OpAtomicFlagTestAndSet, 318, 6);

        SPIRV_Op(OpAtomicFlagClear, 319, 4);

        SPIRV_OpVar(OpImageSparseRead, 320, 5);

        //Version >= 1.1
        SPIRV_Op(OpSizeOf, 321, 4);

        //Version >= 1.1
        SPIRV_Op(OpTypePipeStorage, 322, 3);

        //Version >= 1.1
        SPIRV_Op(OpConstantPipeStorage, 323, 6);

        //Version >= 1.1
        SPIRV_Op(OpCreatePipeFromPipeStorage, 324, 4);

        //Version >= 1.1
        SPIRV_Op(OpGetKernelLocalSizeForSubgroupCount, 325, 8);

        //Version >= 1.1
        SPIRV_Op(OpGetKernelMaxNumSubgroups, 326, 7);

        //Version >= 1.1
        SPIRV_Op(OpTypeNamedBarrier, 327, 3);

        //Version >= 1.1
        SPIRV_Op(OpNamedBarrierInitialize, 328, 4);

        //Version >= 1.1
        SPIRV_Op(OpMemoryNamedBarrier, 329, 4);

        //Version >= 1.1
        SPIRV_OpVar(OpModuleProcessed, 330, 2);

        //Version >= 1.2
        SPIRV_OpVar(OpExecutionModeId, 331, 3);

        //Version >= 1.2
        SPIRV_OpVar(OpDecorateId, 332, 3);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformElect, 333, 4);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformAll, 334, 5);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformAny, 335, 5);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformAllEqual, 336, 5);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformBroadcast, 337, 6);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformBroadcastFirst, 338, 5);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformBallot, 339, 5);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformInverseBallot, 340, 5);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformBallotBitExtract, 341, 6);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformBallotBitCount, 342, 6);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformBallotFindLSB, 343, 5);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformBallotFindMSB, 344, 5);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformShuffle, 345, 6);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformShuffleXor, 346, 6);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformShuffleUp, 347, 6);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformShuffleDown, 348, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformIAdd, 349, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformFAdd, 350, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformIMul, 351, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformFMul, 352, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformSMin, 353, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformUMin, 354, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformFMin, 355, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformSMax, 356, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformUMax, 357, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformFMax, 358, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformBitwiseAnd, 359, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformBitwiseOr, 360, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformBitwiseXor, 361, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformLogicalAnd, 362, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformLogicalOr, 363, 6);

        //Version >= 1.3
        SPIRV_OpVar(OpGroupNonUniformLogicalXor, 364, 6);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformQuadBroadcast, 365, 6);

        //Version >= 1.3
        SPIRV_Op(OpGroupNonUniformQuadSwap, 366, 6);

        SPIRV_Op(OpCopyLogical, 400, 4);

        //Version >= 1.4
        SPIRV_Op(OpPtrEqual, 401, 5);

        //Version >= 1.4
        SPIRV_Op(OpPtrNotEqual, 402, 5);

        //Version >= 1.4
        SPIRV_Op(OpPtrDiff, 403, 5);

        //Version >= 1.2
        SPIRV_OpVar(OpMemberDecorateString, 5633, 5);

    }
	
}
