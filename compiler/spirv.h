
#pragma once

#include <stdint.h>

#define SPIRV_OP SpvOp inline

namespace caliburn
{
    typedef uint32_t SpvVar;

    namespace spirv
    {
        struct SpvOp
        {
            uint32_t rawInstruction = 0;

            SpvOp() {}

            SpvOp(uint32_t code)
            {
                rawInstruction = code;
            }

            SpvOp(uint32_t wordCount, uint32_t opCode)
            {
                rawInstruction = ((wordCount & 0xFFFF) << 16) | (opCode & 0xFFFF);
            }

            operator uint32_t()
            {
                return rawInstruction;
            }

            uint32_t words()
            {
                return (rawInstruction & (0xFFFF << 16)) >> 16;
            }

            uint32_t opCode()
            {
                return rawInstruction & 0xFFFF;
            }

        };

        uint32_t inline MagicNumber()
        {
            return 0x07230203;
        }

        uint32_t inline Version(int major, int minor)
        {
            return ((major & 0xFF) << 16) | ((minor & 0xFF) << 8);
        }

        enum class Capability : uint32_t
        {
            MATRIX =            0,
            SHADER =            1,
            GEOMETRY =          2,
            TESSELLATION =      3,
            ADDRESSES =         4,
            LINKAGE =           5,
            KERNEL =            6,
            VECTOR16 =          7,
            FLOAT16BUFFER =     8,
            FLOAT16 =           9,
            FLOAT64 =           10,
            INT64 =             11,
            INT64ATOMICS =      12,
            IMAGEBASIC =        13,
            IMAGEREADWRITE =    14,
            IMAGEMIPMAP =       15,
            PIPES =             17,
            GROUPS =            18,
            DEVICEQNEUEUE =     19,
            LITERALSAMPLER =    20,
            ATOMICSTORAGE =     21,
            INT16 =             22
        };

        enum class AddressingModel : uint32_t
        {
            LOGICAL =                   0,
            PHYSICAL32 =                1,
            PHYSICAL64 =                2,
            PHYSICALSTORAGEBUFFER64 =   5348
        };

        enum FuncControl : uint32_t
        {
            NONE =          0x00,
            INLINE =        0x01,
            DONTINLINE =    0x02,
            PURE =          0x04,
            CONST =         0x08
        };

        //Not complete, missing a lot of RT extension enums
        enum class StorageClass : uint32_t
        {
            UNIFORM_CONSTANT =  0,
            INPUT =             1,
            UNIFORM =           2,
            OUTPUT =            3,
            WORKGROUP =         4,
            CROSS_WORKGROUP =   5,
            PRIVATE =           6,
            FUNCTION =          7,
            GENERIC =           8,
            PUSH_CONSTANT =     9,
            ATOMIC_COUNTER =    10,
            IMAGE =             11,
            //version >= 1.3
            STORAGE_BUFFER =    12
        };

        //ONLY INSTRUCTIONS BELOW THIS POINT
        //also they're sorted by opcode
        //sorry

        SPIRV_OP OpNop()
        {
            return SpvOp(1, 0);
        }

        SPIRV_OP OpUndef()
        {
            return SpvOp(3, 1);
        }

        SPIRV_OP OpSourceContinued(SpvVar variable)
        {
            return SpvOp(2 + variable, 2);
        }

        SPIRV_OP OpSource(SpvVar variable)
        {
            return SpvOp(3 + variable, 3);
        }

        SPIRV_OP OpSourceExtension(SpvVar variable)
        {
            return SpvOp(2 + variable, 4);
        }

        SPIRV_OP OpName(SpvVar variable)
        {
            return SpvOp(3 + variable, 5);
        }

        SPIRV_OP OpMemberName(SpvVar variable)
        {
            return SpvOp(4 + variable, 6);
        }

        SPIRV_OP OpString(SpvVar variable)
        {
            return SpvOp(3 + variable, 7);
        }

        SPIRV_OP OpLine()
        {
            return SpvOp(4, 8);
        }

        SPIRV_OP OpExtension(SpvVar variable)
        {
            return SpvOp(2 + variable, 10);
        }

        SPIRV_OP OpExtInstImport(SpvVar variable)
        {
            return SpvOp(3 + variable, 11);
        }

        SPIRV_OP OpExtInst(SpvVar variable)
        {
            return SpvOp(5 + variable, 12);
        }

        SPIRV_OP OpMemoryModel()
        {
            return SpvOp(3, 14);
        }

        SPIRV_OP OpEntryPoint(SpvVar variable)
        {
            return SpvOp(4 + variable, 15);
        }

        SPIRV_OP OpExecutionMode(SpvVar variable)
        {
            return SpvOp(3 + variable, 16);
        }

        SPIRV_OP OpCapability()
        {
            return SpvOp(2, 17);
        }

        SPIRV_OP OpTypeVoid()
        {
            return SpvOp(2, 19);
        }

        SPIRV_OP OpTypeBool()
        {
            return SpvOp(2, 20);
        }

        SPIRV_OP OpTypeInt()
        {
            return SpvOp(4, 21);
        }

        SPIRV_OP OpTypeFloat()
        {
            return SpvOp(3, 22);
        }

        SPIRV_OP OpTypeVector()
        {
            return SpvOp(4, 23);
        }

        SPIRV_OP OpTypeMatrix()
        {
            return SpvOp(4, 24);
        }

        SPIRV_OP OpTypeImage(SpvVar variable)
        {
            return SpvOp(9 + variable, 25);
        }

        SPIRV_OP OpTypeSampler()
        {
            return SpvOp(2, 26);
        }

        SPIRV_OP OpTypeSampledImage()
        {
            return SpvOp(3, 27);
        }

        SPIRV_OP OpTypeArray()
        {
            return SpvOp(4, 28);
        }

        SPIRV_OP OpTypeRuntimeArray()
        {
            return SpvOp(3, 29);
        }

        SPIRV_OP OpTypeStruct(SpvVar variable)
        {
            return SpvOp(2 + variable, 30);
        }

        SPIRV_OP OpTypeOpaque(SpvVar variable)
        {
            return SpvOp(3 + variable, 31);
        }

        SPIRV_OP OpTypePointer()
        {
            return SpvOp(4, 32);
        }

        SPIRV_OP OpTypeFunction(SpvVar variable)
        {
            return SpvOp(3 + variable, 33);
        }

        SPIRV_OP OpTypeDevent()
        {
            return SpvOp(2, 34);
        }

        SPIRV_OP OpTypeDeviceEvent()
        {
            return SpvOp(2, 35);
        }

        SPIRV_OP OpTypeReserveId()
        {
            return SpvOp(2, 36);
        }

        SPIRV_OP OpTypeQueue()
        {
            return SpvOp(2, 37);
        }

        SPIRV_OP OpTypePipe()
        {
            return SpvOp(2, 38);
        }

        SPIRV_OP OpTypeForwardPointer()
        {
            return SpvOp(3, 39);
        }

        SPIRV_OP OpConstantTrue()
        {
            return SpvOp(3, 41);
        }

        SPIRV_OP OpConstantFalse()
        {
            return SpvOp(3, 42);
        }

        SPIRV_OP OpConstant(SpvVar variable)
        {
            return SpvOp(4 + variable, 43);
        }

        SPIRV_OP OpConstantComposite(SpvVar variable)
        {
            return SpvOp(3 + variable, 44);
        }

        SPIRV_OP OpConstantSampler()
        {
            return SpvOp(6, 45);
        }

        SPIRV_OP OpConstantNull()
        {
            return SpvOp(3, 46);
        }

        SPIRV_OP OpSpecConstantTrue()
        {
            return SpvOp(3, 48);
        }

        SPIRV_OP OpSpecConstantFalse()
        {
            return SpvOp(3, 49);
        }

        SPIRV_OP OpSpecConstantFalse(SpvVar variable)
        {
            return SpvOp(4 + variable, 50);
        }

        SPIRV_OP OpSpecConstantComposite(SpvVar variable)
        {
            return SpvOp(5 + variable, 51);
        }

        SPIRV_OP OpSpecConstantOp(SpvVar variable)
        {
            return SpvOp(4 + variable, 52);
        }

        SPIRV_OP OpFunction()
        {
            return SpvOp(5, 54);
        }

        SPIRV_OP OpFunctionParameter()
        {
            return SpvOp(3, 55);
        }

        SPIRV_OP OpFunctionEnd()
        {
            return SpvOp(1, 56);
        }

        SPIRV_OP OpFunctionCall(SpvVar variable)
        {
            return SpvOp(4 + variable, 57);
        }

        SPIRV_OP OpVariable(SpvVar variable)
        {
            return SpvOp(4 + variable, 59);
        }

        SPIRV_OP OpImageTexelPointer()
        {
            return SpvOp(6, 60);
        }

        SPIRV_OP OpLoad(SpvVar variable)
        {
            return SpvOp(4 + variable, 61);
        }

        SPIRV_OP OpStore(SpvVar variable)
        {
            return SpvOp(3 + variable, 62);
        }

        SPIRV_OP OpCopyMemory(SpvVar variable)
        {
            return SpvOp(3 + variable, 63);
        }

        SPIRV_OP OpCopyMemorySized(SpvVar variable)
        {
            return SpvOp(4 + variable, 64);
        }

        SPIRV_OP OpAccessChain(SpvVar variable)
        {
            return SpvOp(4 + variable, 65);
        }

        //Execute opcode 66
        SPIRV_OP OpInBoundsAccessChain(SpvVar variable)
        {
            return SpvOp(4 + variable, 66);
        }

        SPIRV_OP OpPtrAccessChain(SpvVar variable)
        {
            return SpvOp(5 + variable, 67);
        }

        SPIRV_OP OpArrayLength()
        {
            return SpvOp(5, 68);
        }

        //nice
        SPIRV_OP OpGenericPtrMemSemantics()
        {
            return SpvOp(4, 69);
        }

        SPIRV_OP OpInBoundsPtrAccessChain(SpvVar variable)
        {
            return SpvOp(5 + variable, 70);
        }

        SPIRV_OP OpDecorate(SpvVar variable)
        {
            return SpvOp(3 + variable, 71);
        }

        SPIRV_OP OpMemberDecorate(SpvVar variable)
        {
            return SpvOp(4 + variable, 72);
        }

        SPIRV_OP OpDecorationGroup()
        {
            return SpvOp(2, 73);
        }

        SPIRV_OP OpGroupDecorate(SpvVar variable)
        {
            return SpvOp(2 + variable, 74);
        }

        SPIRV_OP OpGroupMemberDecorate(SpvVar variable)
        {
            return SpvOp(2 + variable, 75);
        }

        SPIRV_OP OpVectorExtractDynamic()
        {
            return SpvOp(5, 77);
        }

        SPIRV_OP OpVectorInsertDynamic()
        {
            return SpvOp(6, 78);
        }

        SPIRV_OP OpVectorShuffle(SpvVar variable)
        {
            return SpvOp(5 + variable, 79);
        }

        SPIRV_OP OpCompositeConstruct(SpvVar variable)
        {
            return SpvOp(3 + variable, 80);
        }

        SPIRV_OP OpCompositeExtract(SpvVar variable)
        {
            return SpvOp(4 + variable, 81);
        }

        SPIRV_OP OpCompositeInsert(SpvVar variable)
        {
            return SpvOp(5 + variable, 82);
        }

        SPIRV_OP OpCopyObject()
        {
            return SpvOp(4, 83);
        }

        SPIRV_OP OpTranspose()
        {
            return SpvOp(4, 84);
        }

        SPIRV_OP OpSampledImage()
        {
            return SpvOp(5, 86);
        }

        SPIRV_OP OpImageSampleImplicitLod(SpvVar variable)
        {
            return SpvOp(5 + variable, 87);
        }

        SPIRV_OP OpImageSampleExplicitLod(SpvVar variable)
        {
            return SpvOp(7 + variable, 88);
        }

        SPIRV_OP OpImageSampleDrefImplicitLod(SpvVar variable)
        {
            return SpvOp(6 + variable, 89);
        }

        SPIRV_OP OpImageSampleDrefExplicitLod(SpvVar variable)
        {
            return SpvOp(8 + variable, 90);
        }

        SPIRV_OP OpImageSampleProjImplicitLod(SpvVar variable)
        {
            return SpvOp(5 + variable, 91);
        }

        SPIRV_OP OpImageSampleProjExplicitLod(SpvVar variable)
        {
            return SpvOp(7 + variable, 92);
        }

        SPIRV_OP OpImageSampleProjDrefImplicitLod(SpvVar variable)
        {
            return SpvOp(6 + variable, 93);
        }

        SPIRV_OP OpImageSampleProjDrefExplicitLod(SpvVar variable)
        {
            return SpvOp(8 + variable, 94);
        }

        SPIRV_OP OpImageFetch(SpvVar variable)
        {
            return SpvOp(5 + variable, 95);
        }

        SPIRV_OP OpImageGather(SpvVar variable)
        {
            return SpvOp(6 + variable, 96);
        }

        SPIRV_OP OpImageDrefGather(SpvVar variable)
        {
            return SpvOp(6 + variable, 97);
        }

        SPIRV_OP OpImageRead(SpvVar variable)
        {
            return SpvOp(5 + variable, 98);
        }

        SPIRV_OP OpImageWrite(SpvVar variable)
        {
            return SpvOp(4 + variable, 99);
        }

        SPIRV_OP OpImage()
        {
            return SpvOp(4, 100);
        }

        SPIRV_OP OpImageQueryFormat()
        {
            return SpvOp(4, 101);
        }

        SPIRV_OP OpImageQueryOrder()
        {
            return SpvOp(4, 102);
        }

        SPIRV_OP OpImageQuerySizeLod()
        {
            return SpvOp(5, 103);
        }

        SPIRV_OP OpImageQuerySize()
        {
            return SpvOp(4, 104);
        }

        SPIRV_OP OpImageQueryLod()
        {
            return SpvOp(5, 105);
        }

        SPIRV_OP OpImageQueryLevels()
        {
            return SpvOp(4, 106);
        }

        SPIRV_OP OpImageQuerySamples()
        {
            return SpvOp(4, 107);
        }

        SPIRV_OP OpConvertFToU()
        {
            return SpvOp(4, 109);
        }

        SPIRV_OP OpConvertFToS()
        {
            return SpvOp(4, 110);
        }

        SPIRV_OP OpConvertSToF()
        {
            return SpvOp(4, 111);
        }

        SPIRV_OP OpConvertUToF()
        {
            return SpvOp(4, 112);
        }

        SPIRV_OP OpUConvert()
        {
            return SpvOp(4, 113);
        }

        SPIRV_OP OpSConvert()
        {
            return SpvOp(4, 114);
        }

        SPIRV_OP OpFConvert()
        {
            return SpvOp(4, 115);
        }

        SPIRV_OP OpQuantizeToF16()
        {
            return SpvOp(4, 116);
        }

        SPIRV_OP OpConvertPtrToU()
        {
            return SpvOp(4, 117);
        }

        SPIRV_OP OpSatConvertSToU()
        {
            return SpvOp(4, 118);
        }

        SPIRV_OP OpSatConvertUToS()
        {
            return SpvOp(4, 119);
        }

        SPIRV_OP OpConvertUToPtr()
        {
            return SpvOp(4, 120);
        }

        SPIRV_OP OpPtrCastToGeneric()
        {
            return SpvOp(4, 121);
        }

        SPIRV_OP OpGenericCastToPtr()
        {
            return SpvOp(4, 122);
        }

        SPIRV_OP OpGenericCastToPtrExplicit()
        {
            return SpvOp(5, 123);
        }

        SPIRV_OP OpBitcast()
        {
            return SpvOp(4, 124);
        }

        SPIRV_OP OpSNegate()
        {
            return SpvOp(4, 126);
        }

        SPIRV_OP OpFNegate()
        {
            return SpvOp(4, 127);
        }

        SPIRV_OP OpIAdd()
        {
            return SpvOp(5, 128);
        }

        SPIRV_OP OpFAdd()
        {
            return SpvOp(5, 129);
        }

        SPIRV_OP OpISub()
        {
            return SpvOp(5, 130);
        }

        SPIRV_OP OpFSub()
        {
            return SpvOp(5, 131);
        }

        SPIRV_OP OpIMul()
        {
            return SpvOp(5, 132);
        }

        SPIRV_OP OpFMul()
        {
            return SpvOp(5, 133);
        }

        SPIRV_OP OpUDiv()
        {
            return SpvOp(5, 134);
        }

        SPIRV_OP OpSDiv()
        {
            return SpvOp(5, 135);
        }

        SPIRV_OP OpFDiv()
        {
            return SpvOp(5, 136);
        }

        SPIRV_OP OpUMod()
        {
            return SpvOp(5, 137);
        }

        SPIRV_OP OpSRem()
        {
            return SpvOp(5, 138);
        }

        SPIRV_OP OpSMod()
        {
            return SpvOp(5, 139);
        }

        SPIRV_OP OpFRem()
        {
            return SpvOp(5, 140);
        }

        SPIRV_OP OpFMod()
        {
            return SpvOp(5, 141);
        }

        SPIRV_OP OpVectorTimesScalar()
        {
            return SpvOp(5, 142);
        }

        SPIRV_OP OpMatrixTimesScalar()
        {
            return SpvOp(5, 143);
        }

        SPIRV_OP OpVectorTimesMatrix()
        {
            return SpvOp(5, 144);
        }

        SPIRV_OP OpMatrixTimesVector()
        {
            return SpvOp(5, 145);
        }

        SPIRV_OP OpMatrixTimesMatrix()
        {
            return SpvOp(5, 146);
        }

        SPIRV_OP OpOuterProduct()
        {
            return SpvOp(5, 147);
        }

        SPIRV_OP OpDot()
        {
            return SpvOp(5, 148);
        }

        SPIRV_OP OpIAddCarry()
        {
            return SpvOp(5, 149);
        }

        SPIRV_OP OpISubBorrow()
        {
            return SpvOp(5, 150);
        }

        SPIRV_OP OpUMulExtended()
        {
            return SpvOp(5, 151);
        }

        SPIRV_OP OpSMulExtended()
        {
            return SpvOp(5, 152);
        }

        SPIRV_OP OpAny()
        {
            return SpvOp(4, 154);
        }

        SPIRV_OP OpAll()
        {
            return SpvOp(4, 155);
        }

        SPIRV_OP OpIsNan()
        {
            return SpvOp(4, 156);
        }

        SPIRV_OP OpIsInf()
        {
            return SpvOp(4, 157);
        }

        SPIRV_OP OpIsFinite()
        {
            return SpvOp(4, 158);
        }

        SPIRV_OP OpIsNormal()
        {
            return SpvOp(4, 159);
        }

        SPIRV_OP OpSignBitSet()
        {
            return SpvOp(4, 160);
        }

        //Deprecated
        SPIRV_OP OpLessOrGreater()
        {
            return SpvOp(5, 161);
        }

        SPIRV_OP OpOrdered()
        {
            return SpvOp(5, 162);
        }

        SPIRV_OP OpUnordered()
        {
            return SpvOp(5, 163);
        }

        SPIRV_OP OpLogicalEqual()
        {
            return SpvOp(5, 164);
        }

        SPIRV_OP OpLogicalNotEqual()
        {
            return SpvOp(5, 165);
        }

        SPIRV_OP OpLogicalOr()
        {
            return SpvOp(5, 166);
        }

        SPIRV_OP OpLogicalAnd()
        {
            return SpvOp(5, 167);
        }

        SPIRV_OP OpLogicalNot()
        {
            return SpvOp(4, 168);
        }

        SPIRV_OP OpSelect()
        {
            return SpvOp(6, 169);
        }

        SPIRV_OP OpIEqual()
        {
            return SpvOp(5, 170);
        }

        SPIRV_OP OpINotEqual()
        {
            return SpvOp(5, 171);
        }

        SPIRV_OP OpUGreaterThan()
        {
            return SpvOp(5, 172);
        }

        SPIRV_OP OpSGreaterThan()
        {
            return SpvOp(5, 173);
        }

        SPIRV_OP OpUGreaterThanEqual()
        {
            return SpvOp(5, 174);
        }

        SPIRV_OP OpSGreaterThanEqual()
        {
            return SpvOp(5, 175);
        }

        SPIRV_OP OpULessThan()
        {
            return SpvOp(5, 176);
        }

        SPIRV_OP OpSLessThan()
        {
            return SpvOp(5, 177);
        }

        SPIRV_OP OpULessThanEqual()
        {
            return SpvOp(5, 178);
        }

        SPIRV_OP OpSLessThanEqual()
        {
            return SpvOp(5, 179);
        }

        SPIRV_OP OpFOrdEqual()
        {
            return SpvOp(5, 180);
        }

        SPIRV_OP OpFUnordEqual()
        {
            return SpvOp(5, 181);
        }

        SPIRV_OP OpFOrdNotEqual()
        {
            return SpvOp(5, 182);
        }

        SPIRV_OP OpFUnordNotEqual()
        {
            return SpvOp(5, 183);
        }

        SPIRV_OP OpFOrdLessThan()
        {
            return SpvOp(5, 184);
        }

        SPIRV_OP OpFUnordLessThan()
        {
            return SpvOp(5, 185);
        }

        SPIRV_OP OpFOrdGreaterThan()
        {
            return SpvOp(5, 186);
        }

        SPIRV_OP OpFUnordGreaterThan()
        {
            return SpvOp(5, 187);
        }

        SPIRV_OP OpFOrdLessThanEqual()
        {
            return SpvOp(5, 188);
        }

        SPIRV_OP OpFUnordLessThanEqual()
        {
            return SpvOp(5, 189);
        }

        SPIRV_OP OpFOrdGreaterThanEqual()
        {
            return SpvOp(5, 190);
        }

        SPIRV_OP OpFUnordGreaterThanEqual()
        {
            return SpvOp(5, 191);
        }

        SPIRV_OP OpShiftRightLogical()
        {
            return SpvOp(5, 194);
        }

        SPIRV_OP OpShiftRightArithmetic()
        {
            return SpvOp(5, 195);
        }

        SPIRV_OP OpShiftLeftLogical()
        {
            return SpvOp(5, 196);
        }

        SPIRV_OP OpBitwiseOr()
        {
            return SpvOp(5, 197);
        }

        SPIRV_OP OpBitwiseXor()
        {
            return SpvOp(5, 198);
        }

        SPIRV_OP OpBitwiseAnd()
        {
            return SpvOp(5, 199);
        }

        SPIRV_OP OpNot()
        {
            return SpvOp(4, 200);
        }

        SPIRV_OP OpBitFieldInsert()
        {
            return SpvOp(7, 201);
        }

        SPIRV_OP OpBitFieldSExtract()
        {
            return SpvOp(6, 202);
        }

        SPIRV_OP OpBitFieldUExtract()
        {
            return SpvOp(6, 203);
        }

        SPIRV_OP OpBitReverse()
        {
            return SpvOp(4, 204);
        }

        SPIRV_OP OpBitCount()
        {
            return SpvOp(4, 205);
        }

        SPIRV_OP OpDPdx()
        {
            return SpvOp(4, 207);
        }

        SPIRV_OP OpDPdy()
        {
            return SpvOp(4, 208);
        }

        SPIRV_OP OpFwidth()
        {
            return SpvOp(4, 209);
        }

        SPIRV_OP OpDPdxFine()
        {
            return SpvOp(4, 210);
        }

        SPIRV_OP OpDPdyFine()
        {
            return SpvOp(4, 211);
        }

        SPIRV_OP OpFwidthFine()
        {
            return SpvOp(4, 212);
        }

        SPIRV_OP OpDPdxCoarse()
        {
            return SpvOp(4, 213);
        }

        SPIRV_OP OpDPdyCoarse()
        {
            return SpvOp(4, 214);
        }

        SPIRV_OP OpFwidthCoarse()
        {
            return SpvOp(4, 215);
        }

        SPIRV_OP OpEmitVertex()
        {
            return SpvOp(1, 218);
        }

        SPIRV_OP OpEndPrimitive()
        {
            return SpvOp(1, 219);
        }

        SPIRV_OP OpEmitStreamVertex()
        {
            return SpvOp(1, 220);
        }

        SPIRV_OP OpEndStreamPrimitive()
        {
            return SpvOp(2, 221);
        }

        SPIRV_OP OpControlBarrier()
        {
            return SpvOp(4, 224);
        }

        SPIRV_OP OpMemoryBarrier()
        {
            return SpvOp(4, 225);
        }

        SPIRV_OP OpAtomicLoad()
        {
            return SpvOp(6, 227);
        }

        SPIRV_OP OpAtomicStore()
        {
            return SpvOp(5, 228);
        }

        SPIRV_OP OpAtomicExchange()
        {
            return SpvOp(7, 229);
        }

        SPIRV_OP OpAtomicCompareExchange()
        {
            return SpvOp(9, 230);
        }

        SPIRV_OP OpAtomicCompareExchangeWeak()
        {
            return SpvOp(9, 231);
        }

        SPIRV_OP OpAtomicIIncrement()
        {
            return SpvOp(6, 232);
        }

        SPIRV_OP OpAtomicIDecrement()
        {
            return SpvOp(6, 233);
        }

        SPIRV_OP OpAtomicIAdd()
        {
            return SpvOp(7, 234);
        }

        SPIRV_OP OpAtomicISub()
        {
            return SpvOp(7, 235);
        }

        SPIRV_OP OpAtomicSMin()
        {
            return SpvOp(7, 236);
        }

        SPIRV_OP OpAtomicUMin()
        {
            return SpvOp(7, 237);
        }

        SPIRV_OP OpAtomicSMax()
        {
            return SpvOp(7, 238);
        }

        SPIRV_OP OpAtomicUMax()
        {
            return SpvOp(7, 239);
        }

        SPIRV_OP OpAtomicAnd()
        {
            return SpvOp(7, 240);
        }

        SPIRV_OP OpAtomicOr()
        {
            return SpvOp(7, 241);
        }

        SPIRV_OP OpAtomicXor()
        {
            return SpvOp(7, 242);
        }

        SPIRV_OP OpPhi(SpvVar variable)
        {
            return SpvOp(3 + variable, 245);
        }

        SPIRV_OP OpLoopMerge(SpvVar variable)
        {
            return SpvOp(4 + variable, 246);
        }

        SPIRV_OP OpSelectionMerge()
        {
            return SpvOp(3, 247);
        }

        SPIRV_OP OpLabel()
        {
            return SpvOp(2, 248);
        }

        SPIRV_OP OpBranch()
        {
            return SpvOp(2, 249);
        }

        SPIRV_OP OpBranchConditional(SpvVar variable)
        {
            return SpvOp(4 + variable, 250);
        }

        SPIRV_OP OpSwitch(SpvVar variable)
        {
            return SpvOp(3 + variable, 251);
        }

        SPIRV_OP OpKill()
        {
            return SpvOp(1, 252);
        }

        SPIRV_OP OpReturn()
        {
            return SpvOp(1, 253);
        }

        SPIRV_OP OpReturnValue()
        {
            return SpvOp(2, 254);
        }

        SPIRV_OP OpUnreachable()
        {
            return SpvOp(1, 255);
        }

        SPIRV_OP OpLifetimeStart()
        {
            return SpvOp(3, 256);
        }

        SPIRV_OP OpLifetimeStop()
        {
            return SpvOp(3, 257);
        }

        SPIRV_OP OpGroupAsyncCopy()
        {
            return SpvOp(9, 259);
        }

        SPIRV_OP OpGroupWaitEvents()
        {
            return SpvOp(4, 260);
        }

        SPIRV_OP OpGroupAll()
        {
            return SpvOp(5, 261);
        }

        SPIRV_OP OpGroupAny()
        {
            return SpvOp(5, 262);
        }

        SPIRV_OP OpGroupBroadcast()
        {
            return SpvOp(6, 263);
        }

        SPIRV_OP OpGroupIAdd()
        {
            return SpvOp(6, 264);
        }

        SPIRV_OP OpGroupFAdd()
        {
            return SpvOp(6, 265);
        }

        SPIRV_OP OpGroupFMin()
        {
            return SpvOp(6, 266);
        }

        SPIRV_OP OpGroupUMin()
        {
            return SpvOp(6, 267);
        }

        SPIRV_OP OpGroupSMin()
        {
            return SpvOp(6, 268);
        }

        SPIRV_OP OpGroupFMax()
        {
            return SpvOp(6, 269);
        }

        SPIRV_OP OpGroupUMax()
        {
            return SpvOp(6, 270);
        }

        SPIRV_OP OpGroupSMax()
        {
            return SpvOp(6, 271);
        }

        SPIRV_OP OpReadPipe()
        {
            return SpvOp(7, 274);
        }

        SPIRV_OP OpWritePipe()
        {
            return SpvOp(7, 275);
        }

        SPIRV_OP OpReservedReadPipe()
        {
            return SpvOp(9, 276);
        }

        SPIRV_OP OpReservedWritePipe()
        {
            return SpvOp(9, 277);
        }

        SPIRV_OP OpReserveReadPipePackets()
        {
            return SpvOp(7, 278);
        }

        SPIRV_OP OpReserveWritePipePackets()
        {
            return SpvOp(7, 279);
        }

        SPIRV_OP OpCommitReadPipe()
        {
            return SpvOp(5, 280);
        }

        SPIRV_OP OpCommitWritePipe()
        {
            return SpvOp(5, 281);
        }

        SPIRV_OP OpIsValidReserveId()
        {
            return SpvOp(4, 282);
        }

        SPIRV_OP OpGetNumPipePackets()
        {
            return SpvOp(5, 283);
        }

        SPIRV_OP OpGetMaxPipePackets()
        {
            return SpvOp(6, 284);
        }

        SPIRV_OP OpGroupReserveReadPipePackets()
        {
            return SpvOp(8, 285);
        }

        SPIRV_OP OpGroupReserveWritePipePackets()
        {
            return SpvOp(8, 286);
        }

        SPIRV_OP OpGroupCommitReadPipe()
        {
            return SpvOp(6, 287);
        }

        SPIRV_OP OpGroupCommitWritePipe()
        {
            return SpvOp(6, 288);
        }

        SPIRV_OP OpEnqueueMarker()
        {
            return SpvOp(7, 291);
        }

        SPIRV_OP OpEnqueueKernel(SpvVar variable)
        {
            return SpvOp(13 + variable, 292);
        }

        SPIRV_OP OpGetKernelNDrangeSubGroupCount()
        {
            return SpvOp(8, 293);
        }

        SPIRV_OP OpGetKernelNDrangeMaxSubGroupSize()
        {
            return SpvOp(8, 294);
        }

        SPIRV_OP OpGetKernelWorkGroupSize()
        {
            return SpvOp(7, 295);
        }

        SPIRV_OP OpGetKernelPreferredWorkGroupSizeMultiple()
        {
            return SpvOp(7, 296);
        }

        SPIRV_OP OpRetainEvent()
        {
            return SpvOp(2, 297);
        }

        SPIRV_OP OpReleaseEvent()
        {
            return SpvOp(2, 298);
        }

        SPIRV_OP OpCreateUserEvent()
        {
            return SpvOp(3, 299);
        }

        SPIRV_OP OpIsValidEvent()
        {
            return SpvOp(4, 300);
        }

        SPIRV_OP OpSetUserEventStatus()
        {
            return SpvOp(3, 301);
        }

        SPIRV_OP OpCaptureEventProfilingInfo()
        {
            return SpvOp(4, 302);
        }

        SPIRV_OP OpGetDefaultQueue()
        {
            return SpvOp(3, 303);
        }

        SPIRV_OP OpBuildNDRange()
        {
            return SpvOp(6, 304);
        }

        SPIRV_OP OpImageQuerySamples(SpvVar variable)
        {
            return SpvOp(5 + variable, 305);
        }

        SPIRV_OP OpImageSparseSampleExplicitLod(SpvVar variable)
        {
            return SpvOp(7 + variable, 306);
        }

        SPIRV_OP OpImageSparseSampleDrefImplicitLod(SpvVar variable)
        {
            return SpvOp(6 + variable, 307);
        }

        SPIRV_OP OpImageSparseSampleDrefExplicitLod(SpvVar variable)
        {
            return SpvOp(8 + variable, 308);
        }

        SPIRV_OP OpImageSparseSampleProjImplicitLod(SpvVar variable)
        {
            return SpvOp(5 + variable, 309);
        }

        SPIRV_OP OpImageSparseSampleProjExplicitLod(SpvVar variable)
        {
            return SpvOp(7 + variable, 310);
        }

        SPIRV_OP OpImageSparseSampleProjDrefImplicitLod(SpvVar variable)
        {
            return SpvOp(6 + variable, 311);
        }

        SPIRV_OP OpImageSparseFetch(SpvVar variable)
        {
            return SpvOp(5 + variable, 313);
        }

        SPIRV_OP OpImageSparseGather(SpvVar variable)
        {
            return SpvOp(6 + variable, 314);
        }

        SPIRV_OP OpImageSparseDrefGather(SpvVar variable)
        {
            return SpvOp(6 + variable, 315);
        }

        SPIRV_OP OpImageSparseDrefGather()
        {
            return SpvOp(4, 316);
        }

        SPIRV_OP OpNoLine()
        {
            return SpvOp(1, 317);
        }

        SPIRV_OP OpAtomicFlagTestAndSet()
        {
            return SpvOp(6, 318);
        }

        SPIRV_OP OpAtomicFlagClear()
        {
            return SpvOp(4, 319);
        }

        SPIRV_OP OpImageSparseRead(SpvVar variable)
        {
            return SpvOp(5 + variable, 320);
        }

        //Version >= 1.1
        SPIRV_OP OpSizeOf()
        {
            return SpvOp(4, 321);
        }

        //Version >= 1.1
        SPIRV_OP OpTypePipeStorage()
        {
            return SpvOp(3, 322);
        }

        //Version >= 1.1
        SPIRV_OP OpConstantPipeStorage()
        {
            return SpvOp(6, 323);
        }

        //Version >= 1.1
        SPIRV_OP OpCreatePipeFromPipeStorage()
        {
            return SpvOp(4, 324);
        }

        //Version >= 1.1
        SPIRV_OP OpGetKernelLocalSizeForSubgroupCount()
        {
            return SpvOp(8, 325);
        }

        //Version >= 1.1
        SPIRV_OP OpGetKernelMaxNumSubgroups()
        {
            return SpvOp(7, 326);
        }

        //Version >= 1.1
        SPIRV_OP OpTypeNamedBarrier()
        {
            return SpvOp(3, 327);
        }

        //Version >= 1.1
        SPIRV_OP OpNamedBarrierInitialize()
        {
            return SpvOp(4, 328);
        }

        //Version >= 1.1
        SPIRV_OP OpMemoryNamedBarrier()
        {
            return SpvOp(4, 329);
        }

        //Version >= 1.1
        SPIRV_OP OpModuleProcessed(SpvVar variable)
        {
            return SpvOp(2 + variable, 330);
        }

        //Version >= 1.2
        SPIRV_OP OpExecutionModeId(SpvVar variable)
        {
            return SpvOp(3 + variable, 331);
        }

        //Version >= 1.2
        SPIRV_OP OpDecorateId(SpvVar variable)
        {
            return SpvOp(3 + variable, 332);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformElect()
        {
            return SpvOp(4, 333);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformAll()
        {
            return SpvOp(5, 334);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformAny()
        {
            return SpvOp(5, 335);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformAllEqual()
        {
            return SpvOp(5, 336);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBroadcast()
        {
            return SpvOp(6, 337);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBroadcastFirst()
        {
            return SpvOp(5, 338);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallot()
        {
            return SpvOp(5, 339);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformInverseBallot()
        {
            return SpvOp(5, 340);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallotBitExtract()
        {
            return SpvOp(6, 341);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallotBitCount()
        {
            return SpvOp(6, 342);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallotFindLSB()
        {
            return SpvOp(5, 343);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallotFindMSB()
        {
            return SpvOp(5, 344);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformShuffle()
        {
            return SpvOp(6, 345);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformShuffleXor()
        {
            return SpvOp(6, 346);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformShuffleUp()
        {
            return SpvOp(6, 347);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformShuffleDown()
        {
            return SpvOp(6, 348);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformIAdd(SpvVar variable)
        {
            return SpvOp(6 + variable, 349);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformFAdd(SpvVar variable)
        {
            return SpvOp(6 + variable, 350);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformIMul(SpvVar variable)
        {
            return SpvOp(6 + variable, 351);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformFMul(SpvVar variable)
        {
            return SpvOp(6 + variable, 352);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformSMin(SpvVar variable)
        {
            return SpvOp(6 + variable, 353);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformUMin(SpvVar variable)
        {
            return SpvOp(6 + variable, 354);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformFMin(SpvVar variable)
        {
            return SpvOp(6 + variable, 355);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformSMax(SpvVar variable)
        {
            return SpvOp(6 + variable, 356);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformUMax(SpvVar variable)
        {
            return SpvOp(6 + variable, 357);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformFMax(SpvVar variable)
        {
            return SpvOp(6 + variable, 358);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBitwiseAnd(SpvVar variable)
        {
            return SpvOp(6 + variable, 359);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBitwiseOr(SpvVar variable)
        {
            return SpvOp(6 + variable, 360);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBitwiseXor(SpvVar variable)
        {
            return SpvOp(6 + variable, 361);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformLogicalAnd(SpvVar variable)
        {
            return SpvOp(6 + variable, 362);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformLogicalOr(SpvVar variable)
        {
            return SpvOp(6 + variable, 363);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformLogicalXor(SpvVar variable)
        {
            return SpvOp(6 + variable, 364);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformQuadBroadcast()
        {
            return SpvOp(6, 365);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformQuadSwap()
        {
            return SpvOp(6, 366);
        }

        SPIRV_OP OpCopyLogical()
        {
            return SpvOp(4, 400);
        }

        //Version >= 1.4
        SPIRV_OP OpPtrEqual()
        {
            return SpvOp(5, 401);
        }

        //Version >= 1.4
        SPIRV_OP OpPtrNotEqual()
        {
            return SpvOp(5, 402);
        }

        //Version >= 1.4
        SPIRV_OP OpPtrDiff()
        {
            return SpvOp(5, 403);
        }

        //Version >= 1.2
        SPIRV_OP OpMemberDecorateString(SpvVar variable)
        {
            return SpvOp(5 + variable, 5633);
        }

    }
	
}
