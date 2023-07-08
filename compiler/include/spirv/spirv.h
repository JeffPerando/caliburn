
#pragma once

#include <stdint.h>
#include <string>

#define SPIRV_Op(Name, ID, Count) constexpr SpvOp inline Name() {return SpvOp(Count, ID);}
#define SPIRV_OpVar(Name, ID, Base) constexpr SpvOp inline Name(uint32_t v = 0) {return SpvOp(Base + v, ID);}

//Pro-tip, future me: For whatever reason, not marking functions as inline in this particular file
//results in linking errors. So... yeah. Fun.

namespace caliburn
{
    namespace spirv
    {
        constexpr uint32_t SPIRV_FILE_MAGIC_NUMBER = 0x07230203;
        //1975 is the year Monty Python and the Holy Grail came out
        constexpr uint32_t CALIBURN_GENERATOR_MAGIC_NUMBER = 1975;

        using SSA = uint32_t;

        inline uint32_t SpvStrLen(std::string str)
        {
            return (uint32_t)((str.length() >> 2) + ((str.length() & 0x3) == 0));
        }

        struct SpvOp
        {
            uint16_t words = 0;
            uint16_t op = 0;

            constexpr SpvOp() = default;
            constexpr SpvOp(uint32_t code)
            {
                words = (code >> 16);
                op = (code & 0xFFFF);
            }
            constexpr SpvOp(uint16_t wordCount, uint16_t opcode) : words(wordCount), op(opcode) {}

            operator uint32_t() const
            {
                return *(uint32_t*)this;
            }

            bool operator<(const SpvOp& rhs) const
            {
                return op < rhs.op;
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
        public:
            Version(uint8_t maj = 1, uint8_t min = 0) : major(maj), minor(min) {}

            operator uint32_t() const
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

            operator uint32_t() const
            {
                return *(uint32_t*)this;
            }

        };

        //Caliburn doesn't use this, this is just for completion's sake.
        //Maybe one day we'll get a spot in this enum...
        enum class SourceLang : uint32_t
        {
            Unknown = 0,
            ESSL = 1,
            GLSL = 2,
            OpenCL_C = 3,
            OpenCL_CPP = 4,
            HLSL = 5,
            CPP_for_OpenCL = 6,
            SYCL = 7
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
            //Got removed at some point
            //ImageSRGB = 16
            Pipes =             17,
            Groups =            18,
            DeviceEnqueue =     19,
            LiteralSampler =    20,
            AtomicStorage =     21,
            Int16 =             22,
            TessellationPointSize = 23,
            GeometryPointSize = 24,
            ImageGatherExtended = 25,
            //StorageImageExtendedFormats  = 26,
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
            ImageMSArray = 48,
            StorageImageExtendedFormats = 49,
            ImageQuery = 50,
            DerivativeControl = 51,
            InterpolationFunction = 52,
            TrasnformFeedback = 53,
            GeometryStreams = 54,
            StorageImageReadWithoutFormat = 55,
            StorageImageWriteWithoutFormat = 56,
            MultiViewport = 57,
            SubgroupDispatch = 58,
            NamedBarrier = 59,
            PipeStorage = 60,
            GroupNonUniform = 61,
            GroupNonUniformVote = 62,
            GroupNonUniformArithmetic = 63,
            GroupNonUniformBallot = 64,
            GroupNonUniformShuffle = 65,
            GroupNonUniformShuffleRelative = 66,
            GroupNonUniformClustered = 67,
            GroupNonUniformQuad = 68,
            ShaderLayer = 69,
            ShaderViewportIndex = 70,
            UniformDecoration = 71,
            FragmentShadingRateKHR = 4422,
            SubgroupBallotKHR = 4423,
            DrawParameters = 4427,
            WorkgroupMemoryExplicitLayoutKHR = 4428,
            WorkgroupMemoryExplicitLayout8BitAccessKHR = 4429,
            WorkgroupMemoryExplicitLayout16BitAccessKHR = 4430,
            SubgroupVoteKHR = 4431,
            StorageBuffer16BitAccess = 4433,
            StorageUniformBufferBlock16 = 4433,//wait, hol up
            UniformAndStorageBuffer16BitAccess = 4434,
            StorageUniform16 = 4434,
            StoragePushConstant16 = 4435,
            StorageInputOutput16 = 4436,
            DeviceGroup = 4437,
            MultiView = 4439,
            VariablePointersStorageBuffer = 4441,
            VariablePointers = 4442,
            AtomicStorageOps = 4445,
            SampleMaskPostDepthCoverage = 4447,
            StorageBuffer8BitAccess = 4448,
            UniformAndStorageBuffer8BitAccess = 4449,
            StoragePushConstant8 = 4450,
            DenormPreserve = 4464,
            DenormFlushToZero = 4465,
            SignedZeroInfNanPreserve = 4466,
            RoundingModeRTE = 4467,
            RoundingModeRTZ = 4468,
            RayQueryProvisionalKHR = 4471,
            RayQueryKHR = 4472,
            RayTraversalPrimitiveCullingKHR = 4478,
            RayTracingKHR = 4479,
            StencilExportEXT = 5013,
            Int64ImageEXT = 5016,
            ShaderClockKHR = 5055,
            ShaderViewportIndexLayerEXT = 5254,
            FragmentFullyCoveredEXT = 5265,
            FragmentBarycentricKHR = 5284,
            FragmentDensityEXT = 5291,
            ShaderNonUniform = 5301,
            RuntimeDescriptorArray = 5302,
            InputAttachmentArrayDynamicIndexing = 5303,
            UniformTexelBufferArrayDynamicIndexing = 5304,
            StorageTexelBufferArrayDynamicIndexing = 5305,
            UniformBufferArrayNonUniformIndexing = 5306,
            SampledImageArrayNonUniformIndexing = 5307,
            StorageBufferArrayNonUniformIndexing = 5308,
            StorageImageArrayNonUniformIndexing = 5309,
            InputAttachmentArrayNonUniformIndexing = 5310,
            UniformTexelBufferArrayNonUniformIndexing = 5311,
            StorageTexelBufferArrayNonUniformIndexing = 5312,
            VulkanMemoryModel = 5345,
            VulkanMemoryModelDeviceScope = 5346,
            PhysicalStorageBufferAddresses = 5347,
            RayTracingProvisionalKHR = 5353,
            FragmentShaderSampleInterlockEXT = 5363,
            FragmentShaderShadingRateInterlockEXT = 5372,
            FragmentShaderPixelInterlockEXT = 5378,
            DemoteToHelperInvocation = 5379,
            ExpectAssumeKHR = 5629,
            DotProductInputAll = 6016,
            DotProductInput4x8Bit = 6017,//Oh hey, DP4a!
            DotProductInput4x8BitPacked = 6018,
            DotProduct = 6019,
            RayCullMaskKHR = 6020,
            BitInstructions = 6025,
            GroupNonUniformRotateKHR = 6026,
            AtomicFloat32AddEXT = 6033,
            AtomicFloat64AddEXT = 6034,
            GroupUniformArithmeticKHR = 6400
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

            /*
            ⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠛⠛⠛⠋⠉⠈⠉⠉⠉⠉⠛⠻⢿⣿⣿⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⣿⡿⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠛⢿⣿⣿⣿⣿
            ⣿⣿⣿⣿⡏⣀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣤⣤⣄⡀⠀⠀⠀⠀⠀⠀⠀⠙⢿⣿⣿
            ⣿⣿⣿⢏⣴⣿⣷⠀⠀⠀⠀⠀⢾⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀⠀⠈⣿⣿
            ⣿⣿⣟⣾⣿⡟⠁⠀⠀⠀⠀⠀⢀⣾⣿⣿⣿⣿⣿⣷⢢⠀⠀⠀⠀⠀⠀⠀⢸⣿
            ⣿⣿⣿⣿⣟⠀⡴⠄⠀⠀⠀⠀⠀⠀⠙⠻⣿⣿⣿⣿⣷⣄⠀⠀⠀⠀⠀⠀⠀⣿
            ⣿⣿⣿⠟⠻⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠶⢴⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⣿
            ⣿⣁⡀⠀⠀⢰⢠⣦⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⣿⣿⣿⣿⣿⡄⠀⣴⣶⣿⡄⣿
            ⣿⡋⠀⠀⠀⠎⢸⣿⡆⠀⠀⠀⠀⠀⠀⣴⣿⣿⣿⣿⣿⣿⣿⠗⢘⣿⣟⠛⠿⣼
            ⣿⣿⠋⢀⡌⢰⣿⡿⢿⡀⠀⠀⠀⠀⠀⠙⠿⣿⣿⣿⣿⣿⡇⠀⢸⣿⣿⣧⢀⣼
            ⣿⣿⣷⢻⠄⠘⠛⠋⠛⠃⠀⠀⠀⠀⠀⢿⣧⠈⠉⠙⠛⠋⠀⠀⠀⣿⣿⣿⣿⣿
            ⣿⣿⣧⠀⠈⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠟⠀⠀⠀⠀⢀⢃⠀⠀⢸⣿⣿⣿⣿
            ⣿⣿⡿⠀⠴⢗⣠⣤⣴⡶⠶⠖⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⡸⠀⣿⣿⣿⣿
            ⣿⣿⣿⡀⢠⣾⣿⠏⠀⠠⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠛⠉⠀⣿⣿⣿⣿
            ⣿⣿⣿⣧⠈⢹⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⣿⣿⣿⣿
            ⣿⣿⣿⣿⡄⠈⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣴⣾⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⣿⣦⣄⣀⣀⣀⣀⠀⠀⠀⠀⠘⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠙⣿⣿⡟⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠇⠀⠁⠀⠀⠹⣿⠃⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⢐⣿⣿⣿⣿⣿⣿⣿⣿⣿
            ⣿⣿⣿⣿⠿⠛⠉⠉⠁⠀⢻⣿⡇⠀⠀⠀⠀⠀⠀⢀⠈⣿⣿⡿⠉⠛⠛⠛⠉⠉
            ⣿⡿⠋⠁⠀⠀⢀⣀⣠⡴⣸⣿⣇⡄⠀⠀⠀⠀⢀⡿⠄⠙⠛⠀⣀⣠⣤⣤⠄⠀
            */
            None = 0xFFFFFFFF
        };
        
        //Nvidia's labels are all the same ID, so we don't bother.
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
            CallableKHR = 5318,
            //I had to dig way too deep to find these
            TaskEXT = 5364,
            MeshEXT = 5365
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

        enum class Scope : uint32_t
        {
            CrossDevice = 0,
            Device = 1,
            Workgroup = 2,
            Subgroup = 3,
            Invocation = 3,
            QueueFamily = 5,
            ShaderCallKHR = 6
        };

        enum class GroupOp : uint32_t
        {
            Reduce = 0,
            InclusiveScan = 1,
            ExclusiveScan = 2
        };

        enum class Decoration : uint32_t
        {
            RelaxedPrecision = 0,
            SpecId = 1,
            Block = 2,
            BufferBlock = 3,
            RowMajor = 4,
            ColMajor = 5,
            ArrayStride = 6,
            MatrixStride = 7,
            GLSLShared = 8,
            GLSLPacked = 9,
            CPacked = 10,
            BuiltIn = 11,
            //no 12???
            NoPerspective = 13,
            Flat = 14,
            Patch = 15,
            Centroid = 16,
            Sample = 17,
            Invariant = 18,
            Restrict = 19,
            Aliased = 20,
            Volatile = 21,
            Constant = 22,
            Coherent = 23,
            NonWritable = 24,//ayo why is this-- nvm
            NonReadable = 25,//...why?
            Uniform = 26,
            UniformId = 27,
            SaturatedConversion = 28,
            Stream = 29,
            Location = 30,
            Component = 31,
            Index = 32,
            Binding = 33,
            DescriptorSet = 34,
            Offset = 35,
            XfbBuffer = 36,
            XfbStride = 37,
            FuncParamAttr = 38,
            FPRoundingMode = 39,
            FPFastMathMode = 40,
            LinkageAttributes = 41,
            NoContraction = 42,
            InputAttachmentIndex = 43,
            Alignment = 44,
            MaxByteOffset = 45,
            AlignmentId = 46,
            MaxByteOffsetId = 47,
            NoSignedWrap = 4469,
            NoUnsignedWrap = 4470,
            //skipping proprietary nonsense...
            PerVertexKHR = 5285,
            NonUniform = 5300,
            RestrictPointer = 5355,
            AliasedPointer = 5356,
            //the above, again...
            CounterBuffer = 5634,
            UserSemantic = 5635
            //why does intel have so many decorations???
        };

        enum class BuiltIn : uint32_t
        {
            Position = 0,
            PointSize = 1,
            //2 is missing???
            ClipDistance = 3,
            CullDistance = 4,
            VertexId = 5,
            InstanceId = 6,
            PrimitiveId = 7,
            InvocationId = 8,
            Layer = 9,
            ViewportIndex = 10,
            TessLevelOuter = 11,
            TessLevelInner = 12,
            TesCoord = 13,
            PatchVertices = 14,
            FragCoord = 15,
            PointCoord = 16,
            FrontFacing = 17,
            SampleId = 18,
            SamplePosition = 19,
            SampleMask = 20,
            //21 is missing...
            FragDepth = 22,
            HelperInvocation = 23,
            NumWorkgroups = 24,
            //deprecated
            WorkgroupSize = 25,
            WorkgroupId = 26,
            LocalInvocationId = 27,
            GlobalInvocationId = 28,
            LocalInvocationIndex = 29,
            WorkDim = 30,
            GlobalSize = 31,
            EnqueuedWorkgroupSize = 32,
            GlobalOffset = 33,
            GlobalLinearId = 34,
            SubgroupSize = 36,
            SubgroupMaxSize = 37,
            NumSubgroups = 38,
            NumEnqueuedSubgroups = 39,
            SubgroupId = 40,
            SubgroupLocalInvocationId = 41,
            VertexIndex = 42,
            InstanceIndex = 43,
            SubgroupEqMask = 4416,
            SubgroupGeMask = 4417,
            SubgroupGtMask = 4418,
            SubgroupLeMask = 4419,
            SubgroupLtMask = 4420,
            BaseVertex = 4424,
            BaseInstance = 4425,
            DrawIndex = 4426,
            PrimitiveShadingRateKHR = 4432

        };

        //see https://registry.khronos.org/SPIR-V/specs/unified1/GLSL.std.450.html
        enum class GLSL450Ext : uint32_t
        {
            None = 0,
            Round = 1,
            RoundEven = 2,
            Trunc = 3,
            FAbs = 4,
            SAbs = 5,
            FSign = 6,
            SSign = 7,
            Floor = 8,
            Ceil = 9,
            Fract = 10,
            Radians = 11,
            Degrees = 12,
            Sin = 13,
            Cos = 14,
            Tan = 15,
            Asin = 16,
            Acos = 17,
            Atan = 18,
            Sinh = 19,
            Cosh = 20,
            Tanh = 21,
            ASinh = 22,
            Acosh = 23,
            Atanh = 24,
            Atan2 = 25,
            Pow = 26,
            Exp = 27,
            Log = 28,
            Exp2 = 29,
            Log2 = 30,
            Sqrt = 31,
            InverseSqrt = 32,
            Determinant = 33,
            MatrixInverse = 34,
            //deprecated
            Modf = 35,
            ModfStruct = 36,
            FMin = 37,
            UMin = 38,
            SMin = 39,
            FMax = 40,
            UMax = 41,
            SMax = 42,
            FClamp = 43,
            UClamp = 44,
            SClamp = 45,
            FMix = 46,
            //47 is removed??
            Step = 48,
            SmoothStep = 49,
            Fma = 50,
            Frexmp = 51,
            FrexpStruct = 52,
            Ldexp = 53,
            PackSnorm4x8 = 54,
            PackUnorm4x8 = 55,
            PackSnorm2x16 = 56,
            PackUnorm2x16 = 57,
            PackHalf2x16 = 58,
            PackDouble2x32 = 59,
            UnpackSnorm2x16 = 60,
            UnpackUnorm2x16 = 61,
            UnpackHalf2x16 = 62,
            UnpackSnorm4x8 = 63,
            UnpackUnorm4x8 = 64,
            UnpackDouble2x32 = 65,
            Length = 66,
            Distance = 67,
            Cross = 68,
            Normalize = 69,
            FaceForward = 70,
            Reflect = 71,
            Refract = 72,
            FindILsb = 73,
            FindSMsb = 74,
            FindUMsb = 75,
            InterpolateAtCentroid = 76,
            InterpolateAtSample = 77,
            InterpolateAtOffset = 78,
            NMin = 79,
            NMax = 80,
            NClamp = 81
        };

        struct ImageOperand
        {
            uint32_t Bias : 1,
                Lod : 1,
                Grad : 1,
                ConstOffset : 1,
                Offset : 1,
                ConstOffsets : 1,
                Sample : 1,
                MinLod : 1,
                MakeTexelAvailable : 1,
                MakeTexelVisible : 1,
                NonPrivateTexel : 1,
                VolatileTexel : 1,
                SignExtend : 1,
                ZeroExtend : 1,
                //Version >= 1.6
                Nontemporal : 1;

            operator uint32_t() const
            {
                return *(uint32_t*)this;
            }

        };

        struct MemorySemantics
        {
            uint32_t Reserved : 1,
                Acquire : 1,
                Release : 1,
                AcquireRelease : 1,
                SequentiallyConsistent : 1,
                UniformMemory : 1,
                SubgroupMemory : 1,
                WorkgroupMemory : 1,
                CrossWorkgroupMemory : 1,
                AtomicCounterMemory : 1,
                ImageMemory : 1,
                OutputMemory : 1,
                MakeAvailable : 1,
                MakeVisible : 1,
                Volatile : 1;

            operator uint32_t() const
            {
                return *(uint32_t*)this;
            }

        };

        struct MemoryOperands
        {
            uint32_t Volatile : 1,
                Aligned : 1,
                Nontemporal : 1,
                MakePointerAvailable : 1,
                MakePointerVisible : 1,
                NonPrivatePointer : 1;

            operator uint32_t() const
            {
                return *(uint32_t*)this;
            }

        };

        struct FPFastMathMode
        {
            uint32_t NotNan : 1,
                NotInf : 1,
                NSZ : 1,
                AllowRecip : 1,
                Fast : 1;

            operator uint32_t() const
            {
                return *(uint32_t*)this;
            }

        };

        struct RayFlags
        {
            uint32_t Opaque : 1,
                NoOpaque : 1,
                TerminateOnFirstHit : 1,
                SkipClosestHitShader : 1,
                CullBackFacingTriangles : 1,
                CullFrontFacingTriangles : 1,
                CullOpaque : 1,
                CullNoOpaque : 1,
                SkipTriangles : 1,
                SkipAABBs : 1;

            operator uint32_t() const
            {
                return *(uint32_t*)this;
            }

        };

        struct FragmentShadingRate
        {
            uint32_t Vertical2Pixels : 1,
                Vertical4Pixels : 1,
                Horizontal2Pixels : 1,
                Horizontal4Pixels : 1;

            operator uint32_t() const
            {
                return *(uint32_t*)this;
            }

        };

        //==========================================
        // SPIR-V opcodes beyond this point
        // also they're sorted by opcode
        // sorry
        //==========================================

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

        //Deprecated
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

        //Version >= 1.6
        SPIRV_Op(OpTerminateInvocation, 4416, 1);

        //Version >= 1.6
        SPIRV_OpVar(OpSDot, 4450, 5);

        //Version >= 1.6
        SPIRV_OpVar(OpUDot, 4451, 5);

        //Version >= 1.6
        SPIRV_OpVar(OpSUDot, 4452, 5);

        //Version >= 1.6
        SPIRV_OpVar(OpSDotAccSat, 4453, 6);

        //Version >= 1.6
        SPIRV_OpVar(OpUDotAccSat, 4454, 6);

        //Version >= 1.6
        SPIRV_OpVar(OpSUDotAccSat, 4455, 6);

        //Version >= 1.6
        SPIRV_Op(OpDemoteToHelperInvocation, 5380, 1);

        //Version >= 1.2
        SPIRV_OpVar(OpMemberDecorateString, 5633, 5);

    }
	
}
