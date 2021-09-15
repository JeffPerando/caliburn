
#pragma once

#include <stdint.h>

#define SPIRV_OP SpvOp inline

namespace caliburn
{
    typedef uint32_t SpvOp;
    typedef uint32_t SpvVar;

    namespace spirv
    {
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

        SPIRV_OP spirvOp(int wordCount, int opCode)
        {
            return ((wordCount & 0xFFFF) << 16) | (opCode & 0xFFFF);
        }

        SPIRV_OP OpNop()
        {
            return spirvOp(1, 0);
        }

        SPIRV_OP OpUndef()
        {
            return spirvOp(3, 1);
        }

        SPIRV_OP OpSourceContinued(SpvVar variable)
        {
            return spirvOp(2 + variable, 2);
        }

        SPIRV_OP OpSource(SpvVar variable)
        {
            return spirvOp(3 + variable, 3);
        }

        SPIRV_OP OpSourceExtension(SpvVar variable)
        {
            return spirvOp(2 + variable, 4);
        }

        SPIRV_OP OpName(SpvVar variable)
        {
            return spirvOp(3 + variable, 5);
        }

        SPIRV_OP OpMemberName(SpvVar variable)
        {
            return spirvOp(4 + variable, 6);
        }

        SPIRV_OP OpString(SpvVar variable)
        {
            return spirvOp(3 + variable, 7);
        }

        SPIRV_OP OpLine()
        {
            return spirvOp(4, 8);
        }

        SPIRV_OP OpExtension(SpvVar variable)
        {
            return spirvOp(2 + variable, 10);
        }

        SPIRV_OP OpExtInstImport(SpvVar variable)
        {
            return spirvOp(3 + variable, 11);
        }

        SPIRV_OP OpExtInst(SpvVar variable)
        {
            return spirvOp(5 + variable, 12);
        }

        SPIRV_OP OpMemoryModel()
        {
            return spirvOp(3, 14);
        }

        SPIRV_OP OpEntryPoint(SpvVar variable)
        {
            return spirvOp(4 + variable, 15);
        }

        SPIRV_OP OpExecutionMode(SpvVar variable)
        {
            return spirvOp(3 + variable, 16);
        }

        SPIRV_OP OpCapability()
        {
            return spirvOp(2, 17);
        }

        SPIRV_OP OpTypeVoid()
        {
            return spirvOp(2, 19);
        }

        SPIRV_OP OpTypeBool()
        {
            return spirvOp(2, 20);
        }

        SPIRV_OP OpTypeInt()
        {
            return spirvOp(4, 21);
        }

        SPIRV_OP OpTypeFloat()
        {
            return spirvOp(3, 22);
        }

        SPIRV_OP OpTypeVector()
        {
            return spirvOp(4, 23);
        }

        SPIRV_OP OpTypeMatrix()
        {
            return spirvOp(4, 24);
        }

        SPIRV_OP OpTypeImage(SpvVar variable)
        {
            return spirvOp(9 + variable, 25);
        }

        SPIRV_OP OpTypeSampler()
        {
            return spirvOp(2, 26);
        }

        SPIRV_OP OpTypeSampledImage()
        {
            return spirvOp(3, 27);
        }

        SPIRV_OP OpTypeArray()
        {
            return spirvOp(4, 28);
        }

        SPIRV_OP OpTypeRuntimeArray()
        {
            return spirvOp(3, 29);
        }

        SPIRV_OP OpTypeStruct(SpvVar variable)
        {
            return spirvOp(2 + variable, 30);
        }

        SPIRV_OP OpTypeOpaque(SpvVar variable)
        {
            return spirvOp(3 + variable, 31);
        }

        SPIRV_OP OpTypePointer()
        {
            return spirvOp(4, 32);
        }

        SPIRV_OP OpTypeFunction(SpvVar variable)
        {
            return spirvOp(3 + variable, 33);
        }

        SPIRV_OP OpTypeDevent()
        {
            return spirvOp(2, 34);
        }

        SPIRV_OP OpTypeDeviceEvent()
        {
            return spirvOp(2, 35);
        }

        SPIRV_OP OpTypeReserveId()
        {
            return spirvOp(2, 36);
        }

        SPIRV_OP OpTypeQueue()
        {
            return spirvOp(2, 37);
        }

        SPIRV_OP OpTypePipe()
        {
            return spirvOp(2, 38);
        }

        SPIRV_OP OpTypeForwardPointer()
        {
            return spirvOp(3, 39);
        }

        SPIRV_OP OpConstantTrue()
        {
            return spirvOp(3, 41);
        }

        SPIRV_OP OpConstantFalse()
        {
            return spirvOp(3, 42);
        }

        SPIRV_OP OpConstant(SpvVar variable)
        {
            return spirvOp(4 + variable, 43);
        }

        SPIRV_OP OpConstantComposite(SpvVar variable)
        {
            return spirvOp(3 + variable, 44);
        }

        SPIRV_OP OpConstantSampler()
        {
            return spirvOp(6, 45);
        }

        SPIRV_OP OpConstantNull()
        {
            return spirvOp(3, 46);
        }

        SPIRV_OP OpSpecConstantTrue()
        {
            return spirvOp(3, 48);
        }

        SPIRV_OP OpSpecConstantFalse()
        {
            return spirvOp(3, 49);
        }

        SPIRV_OP OpSpecConstantFalse(SpvVar variable)
        {
            return spirvOp(4 + variable, 50);
        }

        SPIRV_OP OpSpecConstantComposite(SpvVar variable)
        {
            return spirvOp(5 + variable, 51);
        }

        SPIRV_OP OpSpecConstantOp(SpvVar variable)
        {
            return spirvOp(4 + variable, 52);
        }

        SPIRV_OP OpFunction()
        {
            return spirvOp(5, 54);
        }

        SPIRV_OP OpFunctionParameter()
        {
            return spirvOp(3, 55);
        }

        SPIRV_OP OpFunctionEnd()
        {
            return spirvOp(1, 56);
        }

        SPIRV_OP OpFunctionCall(SpvVar variable)
        {
            return spirvOp(4 + variable, 57);
        }

        SPIRV_OP OpVariable(SpvVar variable)
        {
            return spirvOp(4 + variable, 59);
        }

        SPIRV_OP OpImageTexelPointer()
        {
            return spirvOp(6, 60);
        }

        SPIRV_OP OpLoad(SpvVar variable)
        {
            return spirvOp(4 + variable, 61);
        }

        SPIRV_OP OpStore(SpvVar variable)
        {
            return spirvOp(3 + variable, 62);
        }

        SPIRV_OP OpCopyMemory(SpvVar variable)
        {
            return spirvOp(3 + variable, 63);
        }

        SPIRV_OP OpCopyMemorySized(SpvVar variable)
        {
            return spirvOp(4 + variable, 64);
        }

        SPIRV_OP OpAccessChain(SpvVar variable)
        {
            return spirvOp(4 + variable, 65);
        }

        //Execute opcode 66
        SPIRV_OP OpInBoundsAccessChain(SpvVar variable)
        {
            return spirvOp(4 + variable, 66);
        }

        SPIRV_OP OpPtrAccessChain(SpvVar variable)
        {
            return spirvOp(5 + variable, 67);
        }

        SPIRV_OP OpArrayLength()
        {
            return spirvOp(5, 68);
        }

        //nice
        SPIRV_OP OpGenericPtrMemSemantics()
        {
            return spirvOp(4, 69);
        }

        SPIRV_OP OpInBoundsPtrAccessChain(SpvVar variable)
        {
            return spirvOp(5 + variable, 70);
        }

        SPIRV_OP OpDecorate(SpvVar variable)
        {
            return spirvOp(3 + variable, 71);
        }

        SPIRV_OP OpMemberDecorate(SpvVar variable)
        {
            return spirvOp(4 + variable, 72);
        }

        SPIRV_OP OpDecorationGroup()
        {
            return spirvOp(2, 73);
        }

        SPIRV_OP OpGroupDecorate(SpvVar variable)
        {
            return spirvOp(2 + variable, 74);
        }

        SPIRV_OP OpGroupMemberDecorate(SpvVar variable)
        {
            return spirvOp(2 + variable, 75);
        }

        SPIRV_OP OpVectorExtractDynamic()
        {
            return spirvOp(5, 77);
        }

        SPIRV_OP OpVectorInsertDynamic()
        {
            return spirvOp(6, 78);
        }

        SPIRV_OP OpVectorShuffle(SpvVar variable)
        {
            return spirvOp(5 + variable, 79);
        }

        SPIRV_OP OpCompositeConstruct(SpvVar variable)
        {
            return spirvOp(3 + variable, 80);
        }

        SPIRV_OP OpCompositeExtract(SpvVar variable)
        {
            return spirvOp(4 + variable, 81);
        }

        SPIRV_OP OpCompositeInsert(SpvVar variable)
        {
            return spirvOp(5 + variable, 82);
        }

        SPIRV_OP OpCopyObject()
        {
            return spirvOp(4, 83);
        }

        SPIRV_OP OpTranspose()
        {
            return spirvOp(4, 84);
        }

        SPIRV_OP OpSampledImage()
        {
            return spirvOp(5, 86);
        }

        SPIRV_OP OpImageSampleImplicitLod(SpvVar variable)
        {
            return spirvOp(5 + variable, 87);
        }

        SPIRV_OP OpImageSampleExplicitLod(SpvVar variable)
        {
            return spirvOp(7 + variable, 88);
        }

        SPIRV_OP OpImageSampleDrefImplicitLod(SpvVar variable)
        {
            return spirvOp(6 + variable, 89);
        }

        SPIRV_OP OpImageSampleDrefExplicitLod(SpvVar variable)
        {
            return spirvOp(8 + variable, 90);
        }

        SPIRV_OP OpImageSampleProjImplicitLod(SpvVar variable)
        {
            return spirvOp(5 + variable, 91);
        }

        SPIRV_OP OpImageSampleProjExplicitLod(SpvVar variable)
        {
            return spirvOp(7 + variable, 92);
        }

        SPIRV_OP OpImageSampleProjDrefImplicitLod(SpvVar variable)
        {
            return spirvOp(6 + variable, 93);
        }

        SPIRV_OP OpImageSampleProjDrefExplicitLod(SpvVar variable)
        {
            return spirvOp(8 + variable, 94);
        }

        SPIRV_OP OpImageFetch(SpvVar variable)
        {
            return spirvOp(5 + variable, 95);
        }

        SPIRV_OP OpImageGather(SpvVar variable)
        {
            return spirvOp(6 + variable, 96);
        }

        SPIRV_OP OpImageDrefGather(SpvVar variable)
        {
            return spirvOp(6 + variable, 97);
        }

        SPIRV_OP OpImageRead(SpvVar variable)
        {
            return spirvOp(5 + variable, 98);
        }

        SPIRV_OP OpImageWrite(SpvVar variable)
        {
            return spirvOp(4 + variable, 99);
        }

        SPIRV_OP OpImage()
        {
            return spirvOp(4, 100);
        }

        SPIRV_OP OpImageQueryFormat()
        {
            return spirvOp(4, 101);
        }

        SPIRV_OP OpImageQueryOrder()
        {
            return spirvOp(4, 102);
        }

        SPIRV_OP OpImageQuerySizeLod()
        {
            return spirvOp(5, 103);
        }

        SPIRV_OP OpImageQuerySize()
        {
            return spirvOp(4, 104);
        }

        SPIRV_OP OpImageQueryLod()
        {
            return spirvOp(5, 105);
        }

        SPIRV_OP OpImageQueryLevels()
        {
            return spirvOp(4, 106);
        }

        SPIRV_OP OpImageQuerySamples()
        {
            return spirvOp(4, 107);
        }

        SPIRV_OP OpConvertFToU()
        {
            return spirvOp(4, 109);
        }

        SPIRV_OP OpConvertFToS()
        {
            return spirvOp(4, 110);
        }

        SPIRV_OP OpConvertSToF()
        {
            return spirvOp(4, 111);
        }

        SPIRV_OP OpConvertUToF()
        {
            return spirvOp(4, 112);
        }

        SPIRV_OP OpUConvert()
        {
            return spirvOp(4, 113);
        }

        SPIRV_OP OpSConvert()
        {
            return spirvOp(4, 114);
        }

        SPIRV_OP OpFConvert()
        {
            return spirvOp(4, 115);
        }

        SPIRV_OP OpQuantizeToF16()
        {
            return spirvOp(4, 116);
        }

        SPIRV_OP OpConvertPtrToU()
        {
            return spirvOp(4, 117);
        }

        SPIRV_OP OpSatConvertSToU()
        {
            return spirvOp(4, 118);
        }

        SPIRV_OP OpSatConvertUToS()
        {
            return spirvOp(4, 119);
        }

        SPIRV_OP OpConvertUToPtr()
        {
            return spirvOp(4, 120);
        }

        SPIRV_OP OpPtrCastToGeneric()
        {
            return spirvOp(4, 121);
        }

        SPIRV_OP OpGenericCastToPtr()
        {
            return spirvOp(4, 122);
        }

        SPIRV_OP OpGenericCastToPtrExplicit()
        {
            return spirvOp(5, 123);
        }

        SPIRV_OP OpBitcast()
        {
            return spirvOp(4, 124);
        }

        SPIRV_OP OpSNegate()
        {
            return spirvOp(4, 126);
        }

        SPIRV_OP OpFNegate()
        {
            return spirvOp(4, 127);
        }

        SPIRV_OP OpIAdd()
        {
            return spirvOp(5, 128);
        }

        SPIRV_OP OpFAdd()
        {
            return spirvOp(5, 129);
        }

        SPIRV_OP OpISub()
        {
            return spirvOp(5, 130);
        }

        SPIRV_OP OpFSub()
        {
            return spirvOp(5, 131);
        }

        SPIRV_OP OpIMul()
        {
            return spirvOp(5, 132);
        }

        SPIRV_OP OpFMul()
        {
            return spirvOp(5, 133);
        }

        SPIRV_OP OpUDiv()
        {
            return spirvOp(5, 134);
        }

        SPIRV_OP OpSDiv()
        {
            return spirvOp(5, 135);
        }

        SPIRV_OP OpFDiv()
        {
            return spirvOp(5, 136);
        }

        SPIRV_OP OpUMod()
        {
            return spirvOp(5, 137);
        }

        SPIRV_OP OpSRem()
        {
            return spirvOp(5, 138);
        }

        SPIRV_OP OpSMod()
        {
            return spirvOp(5, 139);
        }

        SPIRV_OP OpFRem()
        {
            return spirvOp(5, 140);
        }

        SPIRV_OP OpFMod()
        {
            return spirvOp(5, 141);
        }

        SPIRV_OP OpVectorTimesScalar()
        {
            return spirvOp(5, 142);
        }

        SPIRV_OP OpMatrixTimesScalar()
        {
            return spirvOp(5, 143);
        }

        SPIRV_OP OpVectorTimesMatrix()
        {
            return spirvOp(5, 144);
        }

        SPIRV_OP OpMatrixTimesVector()
        {
            return spirvOp(5, 145);
        }

        SPIRV_OP OpMatrixTimesMatrix()
        {
            return spirvOp(5, 146);
        }

        SPIRV_OP OpOuterProduct()
        {
            return spirvOp(5, 147);
        }

        SPIRV_OP OpDot()
        {
            return spirvOp(5, 148);
        }

        SPIRV_OP OpIAddCarry()
        {
            return spirvOp(5, 149);
        }

        SPIRV_OP OpISubBorrow()
        {
            return spirvOp(5, 150);
        }

        SPIRV_OP OpUMulExtended()
        {
            return spirvOp(5, 151);
        }

        SPIRV_OP OpSMulExtended()
        {
            return spirvOp(5, 152);
        }

        SPIRV_OP OpAny()
        {
            return spirvOp(4, 154);
        }

        SPIRV_OP OpAll()
        {
            return spirvOp(4, 155);
        }

        SPIRV_OP OpIsNan()
        {
            return spirvOp(4, 156);
        }

        SPIRV_OP OpIsInf()
        {
            return spirvOp(4, 157);
        }

        SPIRV_OP OpIsFinite()
        {
            return spirvOp(4, 158);
        }

        SPIRV_OP OpIsNormal()
        {
            return spirvOp(4, 159);
        }

        SPIRV_OP OpSignBitSet()
        {
            return spirvOp(4, 160);
        }

        //Deprecated
        SPIRV_OP OpLessOrGreater()
        {
            return spirvOp(5, 161);
        }

        SPIRV_OP OpOrdered()
        {
            return spirvOp(5, 162);
        }

        SPIRV_OP OpUnordered()
        {
            return spirvOp(5, 163);
        }

        SPIRV_OP OpLogicalEqual()
        {
            return spirvOp(5, 164);
        }

        SPIRV_OP OpLogicalNotEqual()
        {
            return spirvOp(5, 165);
        }

        SPIRV_OP OpLogicalOr()
        {
            return spirvOp(5, 166);
        }

        SPIRV_OP OpLogicalAnd()
        {
            return spirvOp(5, 167);
        }

        SPIRV_OP OpLogicalNot()
        {
            return spirvOp(4, 168);
        }

        SPIRV_OP OpSelect()
        {
            return spirvOp(6, 169);
        }

        SPIRV_OP OpIEqual()
        {
            return spirvOp(5, 170);
        }

        SPIRV_OP OpINotEqual()
        {
            return spirvOp(5, 171);
        }

        SPIRV_OP OpUGreaterThan()
        {
            return spirvOp(5, 172);
        }

        SPIRV_OP OpSGreaterThan()
        {
            return spirvOp(5, 173);
        }

        SPIRV_OP OpUGreaterThanEqual()
        {
            return spirvOp(5, 174);
        }

        SPIRV_OP OpSGreaterThanEqual()
        {
            return spirvOp(5, 175);
        }

        SPIRV_OP OpULessThan()
        {
            return spirvOp(5, 176);
        }

        SPIRV_OP OpSLessThan()
        {
            return spirvOp(5, 177);
        }

        SPIRV_OP OpULessThanEqual()
        {
            return spirvOp(5, 178);
        }

        SPIRV_OP OpSLessThanEqual()
        {
            return spirvOp(5, 179);
        }

        SPIRV_OP OpFOrdEqual()
        {
            return spirvOp(5, 180);
        }

        SPIRV_OP OpFUnordEqual()
        {
            return spirvOp(5, 181);
        }

        SPIRV_OP OpFOrdNotEqual()
        {
            return spirvOp(5, 182);
        }

        SPIRV_OP OpFUnordNotEqual()
        {
            return spirvOp(5, 183);
        }

        SPIRV_OP OpFOrdLessThan()
        {
            return spirvOp(5, 184);
        }

        SPIRV_OP OpFUnordLessThan()
        {
            return spirvOp(5, 185);
        }

        SPIRV_OP OpFOrdGreaterThan()
        {
            return spirvOp(5, 186);
        }

        SPIRV_OP OpFUnordGreaterThan()
        {
            return spirvOp(5, 187);
        }

        SPIRV_OP OpFOrdLessThanEqual()
        {
            return spirvOp(5, 188);
        }

        SPIRV_OP OpFUnordLessThanEqual()
        {
            return spirvOp(5, 189);
        }

        SPIRV_OP OpFOrdGreaterThanEqual()
        {
            return spirvOp(5, 190);
        }

        SPIRV_OP OpFUnordGreaterThanEqual()
        {
            return spirvOp(5, 191);
        }

        SPIRV_OP OpShiftRightLogical()
        {
            return spirvOp(5, 194);
        }

        SPIRV_OP OpShiftRightArithmetic()
        {
            return spirvOp(5, 195);
        }

        SPIRV_OP OpShiftLeftLogical()
        {
            return spirvOp(5, 196);
        }

        SPIRV_OP OpBitwiseOr()
        {
            return spirvOp(5, 197);
        }

        SPIRV_OP OpBitwiseXor()
        {
            return spirvOp(5, 198);
        }

        SPIRV_OP OpBitwiseAnd()
        {
            return spirvOp(5, 199);
        }

        SPIRV_OP OpNot()
        {
            return spirvOp(4, 200);
        }

        SPIRV_OP OpBitFieldInsert()
        {
            return spirvOp(7, 201);
        }

        SPIRV_OP OpBitFieldSExtract()
        {
            return spirvOp(6, 202);
        }

        SPIRV_OP OpBitFieldUExtract()
        {
            return spirvOp(6, 203);
        }

        SPIRV_OP OpBitReverse()
        {
            return spirvOp(4, 204);
        }

        SPIRV_OP OpBitCount()
        {
            return spirvOp(4, 205);
        }

        SPIRV_OP OpDPdx()
        {
            return spirvOp(4, 207);
        }

        SPIRV_OP OpDPdy()
        {
            return spirvOp(4, 208);
        }

        SPIRV_OP OpFwidth()
        {
            return spirvOp(4, 209);
        }

        SPIRV_OP OpDPdxFine()
        {
            return spirvOp(4, 210);
        }

        SPIRV_OP OpDPdyFine()
        {
            return spirvOp(4, 211);
        }

        SPIRV_OP OpFwidthFine()
        {
            return spirvOp(4, 212);
        }

        SPIRV_OP OpDPdxCoarse()
        {
            return spirvOp(4, 213);
        }

        SPIRV_OP OpDPdyCoarse()
        {
            return spirvOp(4, 214);
        }

        SPIRV_OP OpFwidthCoarse()
        {
            return spirvOp(4, 215);
        }

        SPIRV_OP OpEmitVertex()
        {
            return spirvOp(1, 218);
        }

        SPIRV_OP OpEndPrimitive()
        {
            return spirvOp(1, 219);
        }

        SPIRV_OP OpEmitStreamVertex()
        {
            return spirvOp(1, 220);
        }

        SPIRV_OP OpEndStreamPrimitive()
        {
            return spirvOp(2, 221);
        }

        SPIRV_OP OpControlBarrier()
        {
            return spirvOp(4, 224);
        }

        SPIRV_OP OpMemoryBarrier()
        {
            return spirvOp(4, 225);
        }

        SPIRV_OP OpAtomicLoad()
        {
            return spirvOp(6, 227);
        }

        SPIRV_OP OpAtomicStore()
        {
            return spirvOp(5, 228);
        }

        SPIRV_OP OpAtomicExchange()
        {
            return spirvOp(7, 229);
        }

        SPIRV_OP OpAtomicCompareExchange()
        {
            return spirvOp(9, 230);
        }

        SPIRV_OP OpAtomicCompareExchangeWeak()
        {
            return spirvOp(9, 231);
        }

        SPIRV_OP OpAtomicIIncrement()
        {
            return spirvOp(6, 232);
        }

        SPIRV_OP OpAtomicIDecrement()
        {
            return spirvOp(6, 233);
        }

        SPIRV_OP OpAtomicIAdd()
        {
            return spirvOp(7, 234);
        }

        SPIRV_OP OpAtomicISub()
        {
            return spirvOp(7, 235);
        }

        SPIRV_OP OpAtomicSMin()
        {
            return spirvOp(7, 236);
        }

        SPIRV_OP OpAtomicUMin()
        {
            return spirvOp(7, 237);
        }

        SPIRV_OP OpAtomicSMax()
        {
            return spirvOp(7, 238);
        }

        SPIRV_OP OpAtomicUMax()
        {
            return spirvOp(7, 239);
        }

        SPIRV_OP OpAtomicAnd()
        {
            return spirvOp(7, 240);
        }

        SPIRV_OP OpAtomicOr()
        {
            return spirvOp(7, 241);
        }

        SPIRV_OP OpAtomicXor()
        {
            return spirvOp(7, 242);
        }

        SPIRV_OP OpPhi(SpvVar variable)
        {
            return spirvOp(3 + variable, 245);
        }

        SPIRV_OP OpLoopMerge(SpvVar variable)
        {
            return spirvOp(4 + variable, 246);
        }

        SPIRV_OP OpSelectionMerge()
        {
            return spirvOp(3, 247);
        }

        SPIRV_OP OpLabel()
        {
            return spirvOp(2, 248);
        }

        SPIRV_OP OpBranch()
        {
            return spirvOp(2, 249);
        }

        SPIRV_OP OpBranchConditional(SpvVar variable)
        {
            return spirvOp(4 + variable, 250);
        }

        SPIRV_OP OpSwitch(SpvVar variable)
        {
            return spirvOp(3 + variable, 251);
        }

        SPIRV_OP OpKill()
        {
            return spirvOp(1, 252);
        }

        SPIRV_OP OpReturn()
        {
            return spirvOp(1, 253);
        }

        SPIRV_OP OpReturnValue()
        {
            return spirvOp(2, 254);
        }

        SPIRV_OP OpUnreachable()
        {
            return spirvOp(1, 255);
        }

        SPIRV_OP OpLifetimeStart()
        {
            return spirvOp(3, 256);
        }

        SPIRV_OP OpLifetimeStop()
        {
            return spirvOp(3, 257);
        }

        SPIRV_OP OpGroupAsyncCopy()
        {
            return spirvOp(9, 259);
        }

        SPIRV_OP OpGroupWaitEvents()
        {
            return spirvOp(4, 260);
        }

        SPIRV_OP OpGroupAll()
        {
            return spirvOp(5, 261);
        }

        SPIRV_OP OpGroupAny()
        {
            return spirvOp(5, 262);
        }

        SPIRV_OP OpGroupBroadcast()
        {
            return spirvOp(6, 263);
        }

        SPIRV_OP OpGroupIAdd()
        {
            return spirvOp(6, 264);
        }

        SPIRV_OP OpGroupFAdd()
        {
            return spirvOp(6, 265);
        }

        SPIRV_OP OpGroupFMin()
        {
            return spirvOp(6, 266);
        }

        SPIRV_OP OpGroupUMin()
        {
            return spirvOp(6, 267);
        }

        SPIRV_OP OpGroupSMin()
        {
            return spirvOp(6, 268);
        }

        SPIRV_OP OpGroupFMax()
        {
            return spirvOp(6, 269);
        }

        SPIRV_OP OpGroupUMax()
        {
            return spirvOp(6, 270);
        }

        SPIRV_OP OpGroupSMax()
        {
            return spirvOp(6, 271);
        }

        SPIRV_OP OpReadPipe()
        {
            return spirvOp(7, 274);
        }

        SPIRV_OP OpWritePipe()
        {
            return spirvOp(7, 275);
        }

        SPIRV_OP OpReservedReadPipe()
        {
            return spirvOp(9, 276);
        }

        SPIRV_OP OpReservedWritePipe()
        {
            return spirvOp(9, 277);
        }

        SPIRV_OP OpReserveReadPipePackets()
        {
            return spirvOp(7, 278);
        }

        SPIRV_OP OpReserveWritePipePackets()
        {
            return spirvOp(7, 279);
        }

        SPIRV_OP OpCommitReadPipe()
        {
            return spirvOp(5, 280);
        }

        SPIRV_OP OpCommitWritePipe()
        {
            return spirvOp(5, 281);
        }

        SPIRV_OP OpIsValidReserveId()
        {
            return spirvOp(4, 282);
        }

        SPIRV_OP OpGetNumPipePackets()
        {
            return spirvOp(5, 283);
        }

        SPIRV_OP OpGetMaxPipePackets()
        {
            return spirvOp(6, 284);
        }

        SPIRV_OP OpGroupReserveReadPipePackets()
        {
            return spirvOp(8, 285);
        }

        SPIRV_OP OpGroupReserveWritePipePackets()
        {
            return spirvOp(8, 286);
        }

        SPIRV_OP OpGroupCommitReadPipe()
        {
            return spirvOp(6, 287);
        }

        SPIRV_OP OpGroupCommitWritePipe()
        {
            return spirvOp(6, 288);
        }

        SPIRV_OP OpEnqueueMarker()
        {
            return spirvOp(7, 291);
        }

        SPIRV_OP OpEnqueueKernel(SpvVar variable)
        {
            return spirvOp(13 + variable, 292);
        }

        SPIRV_OP OpGetKernelNDrangeSubGroupCount()
        {
            return spirvOp(8, 293);
        }

        SPIRV_OP OpGetKernelNDrangeMaxSubGroupSize()
        {
            return spirvOp(8, 294);
        }

        SPIRV_OP OpGetKernelWorkGroupSize()
        {
            return spirvOp(7, 295);
        }

        SPIRV_OP OpGetKernelPreferredWorkGroupSizeMultiple()
        {
            return spirvOp(7, 296);
        }

        SPIRV_OP OpRetainEvent()
        {
            return spirvOp(2, 297);
        }

        SPIRV_OP OpReleaseEvent()
        {
            return spirvOp(2, 298);
        }

        SPIRV_OP OpCreateUserEvent()
        {
            return spirvOp(3, 299);
        }

        SPIRV_OP OpIsValidEvent()
        {
            return spirvOp(4, 300);
        }

        SPIRV_OP OpSetUserEventStatus()
        {
            return spirvOp(3, 301);
        }

        SPIRV_OP OpCaptureEventProfilingInfo()
        {
            return spirvOp(4, 302);
        }

        SPIRV_OP OpGetDefaultQueue()
        {
            return spirvOp(3, 303);
        }

        SPIRV_OP OpBuildNDRange()
        {
            return spirvOp(6, 304);
        }

        SPIRV_OP OpImageQuerySamples(SpvVar variable)
        {
            return spirvOp(5 + variable, 305);
        }

        SPIRV_OP OpImageSparseSampleExplicitLod(SpvVar variable)
        {
            return spirvOp(7 + variable, 306);
        }

        SPIRV_OP OpImageSparseSampleDrefImplicitLod(SpvVar variable)
        {
            return spirvOp(6 + variable, 307);
        }

        SPIRV_OP OpImageSparseSampleDrefExplicitLod(SpvVar variable)
        {
            return spirvOp(8 + variable, 308);
        }

        SPIRV_OP OpImageSparseSampleProjImplicitLod(SpvVar variable)
        {
            return spirvOp(5 + variable, 309);
        }

        SPIRV_OP OpImageSparseSampleProjExplicitLod(SpvVar variable)
        {
            return spirvOp(7 + variable, 310);
        }

        SPIRV_OP OpImageSparseSampleProjDrefImplicitLod(SpvVar variable)
        {
            return spirvOp(6 + variable, 311);
        }

        SPIRV_OP OpImageSparseFetch(SpvVar variable)
        {
            return spirvOp(5 + variable, 313);
        }

        SPIRV_OP OpImageSparseGather(SpvVar variable)
        {
            return spirvOp(6 + variable, 314);
        }

        SPIRV_OP OpImageSparseDrefGather(SpvVar variable)
        {
            return spirvOp(6 + variable, 315);
        }

        SPIRV_OP OpImageSparseDrefGather()
        {
            return spirvOp(4, 316);
        }

        SPIRV_OP OpNoLine()
        {
            return spirvOp(1, 317);
        }

        SPIRV_OP OpAtomicFlagTestAndSet()
        {
            return spirvOp(6, 318);
        }

        SPIRV_OP OpAtomicFlagClear()
        {
            return spirvOp(4, 319);
        }

        SPIRV_OP OpImageSparseRead(SpvVar variable)
        {
            return spirvOp(5 + variable, 320);
        }

        //Version >= 1.1
        SPIRV_OP OpSizeOf()
        {
            return spirvOp(4, 321);
        }

        //Version >= 1.1
        SPIRV_OP OpTypePipeStorage()
        {
            return spirvOp(3, 322);
        }

        //Version >= 1.1
        SPIRV_OP OpConstantPipeStorage()
        {
            return spirvOp(6, 323);
        }

        //Version >= 1.1
        SPIRV_OP OpCreatePipeFromPipeStorage()
        {
            return spirvOp(4, 324);
        }

        //Version >= 1.1
        SPIRV_OP OpGetKernelLocalSizeForSubgroupCount()
        {
            return spirvOp(8, 325);
        }

        //Version >= 1.1
        SPIRV_OP OpGetKernelMaxNumSubgroups()
        {
            return spirvOp(7, 326);
        }

        //Version >= 1.1
        SPIRV_OP OpTypeNamedBarrier()
        {
            return spirvOp(3, 327);
        }

        //Version >= 1.1
        SPIRV_OP OpNamedBarrierInitialize()
        {
            return spirvOp(4, 328);
        }

        //Version >= 1.1
        SPIRV_OP OpMemoryNamedBarrier()
        {
            return spirvOp(4, 329);
        }

        //Version >= 1.1
        SPIRV_OP OpModuleProcessed(SpvVar variable)
        {
            return spirvOp(2 + variable, 330);
        }

        //Version >= 1.2
        SPIRV_OP OpExecutionModeId(SpvVar variable)
        {
            return spirvOp(3 + variable, 331);
        }

        //Version >= 1.2
        SPIRV_OP OpDecorateId(SpvVar variable)
        {
            return spirvOp(3 + variable, 332);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformElect()
        {
            return spirvOp(4, 333);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformAll()
        {
            return spirvOp(5, 334);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformAny()
        {
            return spirvOp(5, 335);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformAllEqual()
        {
            return spirvOp(5, 336);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBroadcast()
        {
            return spirvOp(6, 337);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBroadcastFirst()
        {
            return spirvOp(5, 338);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallot()
        {
            return spirvOp(5, 339);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformInverseBallot()
        {
            return spirvOp(5, 340);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallotBitExtract()
        {
            return spirvOp(6, 341);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallotBitCount()
        {
            return spirvOp(6, 342);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallotFindLSB()
        {
            return spirvOp(5, 343);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBallotFindMSB()
        {
            return spirvOp(5, 344);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformShuffle()
        {
            return spirvOp(6, 345);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformShuffleXor()
        {
            return spirvOp(6, 346);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformShuffleUp()
        {
            return spirvOp(6, 347);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformShuffleDown()
        {
            return spirvOp(6, 348);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformIAdd(SpvVar variable)
        {
            return spirvOp(6 + variable, 349);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformFAdd(SpvVar variable)
        {
            return spirvOp(6 + variable, 350);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformIMul(SpvVar variable)
        {
            return spirvOp(6 + variable, 351);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformFMul(SpvVar variable)
        {
            return spirvOp(6 + variable, 352);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformSMin(SpvVar variable)
        {
            return spirvOp(6 + variable, 353);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformUMin(SpvVar variable)
        {
            return spirvOp(6 + variable, 354);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformFMin(SpvVar variable)
        {
            return spirvOp(6 + variable, 355);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformSMax(SpvVar variable)
        {
            return spirvOp(6 + variable, 356);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformUMax(SpvVar variable)
        {
            return spirvOp(6 + variable, 357);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformFMax(SpvVar variable)
        {
            return spirvOp(6 + variable, 358);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBitwiseAnd(SpvVar variable)
        {
            return spirvOp(6 + variable, 359);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBitwiseOr(SpvVar variable)
        {
            return spirvOp(6 + variable, 360);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformBitwiseXor(SpvVar variable)
        {
            return spirvOp(6 + variable, 361);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformLogicalAnd(SpvVar variable)
        {
            return spirvOp(6 + variable, 362);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformLogicalOr(SpvVar variable)
        {
            return spirvOp(6 + variable, 363);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformLogicalXor(SpvVar variable)
        {
            return spirvOp(6 + variable, 364);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformQuadBroadcast()
        {
            return spirvOp(6, 365);
        }

        //Version >= 1.3
        SPIRV_OP OpGroupNonUniformQuadSwap()
        {
            return spirvOp(6, 366);
        }

        SPIRV_OP OpCopyLogical()
        {
            return spirvOp(4, 400);
        }

        //Version >= 1.4
        SPIRV_OP OpPtrEqual()
        {
            return spirvOp(5, 401);
        }

        //Version >= 1.4
        SPIRV_OP OpPtrNotEqual()
        {
            return spirvOp(5, 402);
        }

        //Version >= 1.4
        SPIRV_OP OpPtrDiff()
        {
            return spirvOp(5, 403);
        }

        //Version >= 1.2
        SPIRV_OP OpMemberDecorateString(SpvVar variable)
        {
            return spirvOp(5 + variable, 5633);
        }

    }
	
}
