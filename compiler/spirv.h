
#pragma once

#include <stdint.h>

namespace caliburn
{
    typedef uint32_t SpvOp;
    typedef uint32_t SpvVar;

    namespace spirv
    {
        uint32_t MagicNumber()
        {
            return 0x07230203;
        }

        //So SPIR-V's version number thing is weird. Quote:
        /*
        The bytes are, high-order to low-order:
        0 | Major Number | Minor Number | 0
        Hence, version 1.3 is the value 0x00010300.
        */
        //what.
        //So given the limited examples... yeah... I wrote this...
        uint32_t Version(int major, int minor)
        {
            return ((major & 0xFFF) << 16) | ((minor & 0xFF) << 8);
        }

        enum SpirVCapability
        {
            MATRIX = 0,
            SHADER = 1,
            GEOMETRY = 2,
            TESSELLATION = 3,
            ADDRESSES = 4,
            LINKAGE = 5,
            KERNEL = 6,
            VECTOR16 = 7,
            FLOAT16BUFFER = 8,
            FLOAT16 = 9,
            FLOAT64 = 10,
            INT64 = 11,
            INT64ATOMICS = 12,
            IMAGEBASIC = 13,
            IMAGEREADWRITE = 14,
            IMAGEMIPMAP = 15,
            PIPES = 17,
            GROUPS = 18,
            DEVICEQNEUEUE = 19,
            LITERALSAMPLER = 20,
            ATOMICSTORAGE = 21,
            INT16 = 22
        };

        enum SpirVAddressingModel
        {
            LOGICAL = 0,
            PHYSICAL32 = 1,
            PHYSICAL64 = 2,
            PHYSICALSTORAGEBUFFER64 = 5348
        };

        //ONLY INSTRUCTIONS BELOW THIS POINT
        //also they're sorted by opcode
        //sorry

        SpvOp spirvOp(int wordCount, int opCode)
        {
            return ((wordCount & 0xFFFF) << 16) | (opCode & 0xFFFF);
        }

        SpvOp OpNop()
        {
            return spirvOp(1, 0);
        }

        SpvOp OpUndef()
        {
            return spirvOp(3, 1);
        }

        SpvOp OpSourceContinued(SpvVar variable)
        {
            return spirvOp(2 + variable, 2);
        }

        SpvOp OpSource(SpvVar variable)
        {
            return spirvOp(3 + variable, 3);
        }

        SpvOp OpSourceExtension(SpvVar variable)
        {
            return spirvOp(2 + variable, 4);
        }

        SpvOp OpName(SpvVar variable)
        {
            return spirvOp(3 + variable, 5);
        }

        SpvOp OpMemberName(SpvVar variable)
        {
            return spirvOp(4 + variable, 6);
        }

        SpvOp OpString(SpvVar variable)
        {
            return spirvOp(3 + variable, 7);
        }

        SpvOp OpLine()
        {
            return spirvOp(4, 8);
        }

        SpvOp OpExtension(SpvVar variable)
        {
            return spirvOp(2 + variable, 10);
        }

        SpvOp OpExtInstImport(SpvVar variable)
        {
            return spirvOp(3 + variable, 11);
        }

        SpvOp OpExtInst(SpvVar variable)
        {
            return spirvOp(5 + variable, 12);
        }

        SpvOp OpMemoryModel()
        {
            return spirvOp(3, 14);
        }

        SpvOp OpEntryPoint(SpvVar variable)
        {
            return spirvOp(4 + variable, 15);
        }

        SpvOp OpExecutionMode(SpvVar variable)
        {
            return spirvOp(3 + variable, 16);
        }

        SpvOp OpCapability()
        {
            return spirvOp(2, 17);
        }

        SpvOp OpTypeVoid()
        {
            return spirvOp(2, 19);
        }

        SpvOp OpTypeBool()
        {
            return spirvOp(2, 20);
        }

        SpvOp OpTypeInt()
        {
            return spirvOp(4, 21);
        }

        SpvOp OpTypeFloat()
        {
            return spirvOp(3, 22);
        }

        SpvOp OpTypeVector()
        {
            return spirvOp(4, 23);
        }

        SpvOp OpTypeMatrix()
        {
            return spirvOp(4, 24);
        }

        SpvOp OpTypeImage(SpvVar variable)
        {
            return spirvOp(9 + variable, 25);
        }

        SpvOp OpTypeSampler()
        {
            return spirvOp(2, 26);
        }

        SpvOp OpTypeSampledImage()
        {
            return spirvOp(3, 27);
        }

        SpvOp OpTypeArray()
        {
            return spirvOp(4, 28);
        }

        SpvOp OpTypeRuntimeArray()
        {
            return spirvOp(3, 29);
        }

        SpvOp OpTypeStruct(SpvVar variable)
        {
            return spirvOp(2 + variable, 30);
        }

        SpvOp OpTypeOpaque(SpvVar variable)
        {
            return spirvOp(3 + variable, 31);
        }

        SpvOp OpTypePointer()
        {
            return spirvOp(4, 32);
        }

        SpvOp OpTypeFunction(SpvVar variable)
        {
            return spirvOp(3 + variable, 33);
        }

        SpvOp OpTypeDevent()
        {
            return spirvOp(2, 34);
        }

        SpvOp OpTypeDeviceEvent()
        {
            return spirvOp(2, 35);
        }

        SpvOp OpTypeReserveId()
        {
            return spirvOp(2, 36);
        }

        SpvOp OpTypeQueue()
        {
            return spirvOp(2, 37);
        }

        SpvOp OpTypePipe()
        {
            return spirvOp(2, 38);
        }

        SpvOp OpTypeForwardPointer()
        {
            return spirvOp(3, 39);
        }

        SpvOp OpConstantTrue()
        {
            return spirvOp(3, 41);
        }

        SpvOp OpConstantFalse()
        {
            return spirvOp(3, 42);
        }

        SpvOp OpConstant(SpvVar variable)
        {
            return spirvOp(4 + variable, 43);
        }

        SpvOp OpConstantComposite(SpvVar variable)
        {
            return spirvOp(3 + variable, 44);
        }

        SpvOp OpConstantSampler()
        {
            return spirvOp(6, 45);
        }

        SpvOp OpConstantNull()
        {
            return spirvOp(3, 46);
        }

        SpvOp OpSpecConstantTrue()
        {
            return spirvOp(3, 48);
        }

        SpvOp OpSpecConstantFalse()
        {
            return spirvOp(3, 49);
        }

        SpvOp OpSpecConstantFalse(SpvVar variable)
        {
            return spirvOp(4 + variable, 50);
        }

        SpvOp OpSpecConstantComposite(SpvVar variable)
        {
            return spirvOp(5 + variable, 51);
        }

        SpvOp OpSpecConstantOp(SpvVar variable)
        {
            return spirvOp(4 + variable, 52);
        }

        SpvOp OpFunction()
        {
            return spirvOp(5, 54);
        }

        SpvOp OpFunctionParameter()
        {
            return spirvOp(3, 55);
        }

        SpvOp OpFunctionEnd()
        {
            return spirvOp(1, 56);
        }

        SpvOp OpFunctionCall(SpvVar variable)
        {
            return spirvOp(4 + variable, 57);
        }

        SpvOp OpVariable(SpvVar variable)
        {
            return spirvOp(4 + variable, 59);
        }

        SpvOp OpImageTexelPointer()
        {
            return spirvOp(6, 60);
        }

        SpvOp OpLoad(SpvVar variable)
        {
            return spirvOp(4 + variable, 61);
        }

        SpvOp OpStore(SpvVar variable)
        {
            return spirvOp(3 + variable, 62);
        }

        SpvOp OpCopyMemory(SpvVar variable)
        {
            return spirvOp(3 + variable, 63);
        }

        SpvOp OpCopyMemorySized(SpvVar variable)
        {
            return spirvOp(4 + variable, 64);
        }

        SpvOp OpAccessChain(SpvVar variable)
        {
            return spirvOp(4 + variable, 65);
        }

        //Execute opcode 66
        SpvOp OpInBoundsAccessChain(SpvVar variable)
        {
            return spirvOp(4 + variable, 66);
        }

        SpvOp OpPtrAccessChain(SpvVar variable)
        {
            return spirvOp(5 + variable, 67);
        }

        SpvOp OpArrayLength()
        {
            return spirvOp(5, 68);
        }

        //nice
        SpvOp OpGenericPtrMemSemantics()
        {
            return spirvOp(4, 69);
        }

        SpvOp OpInBoundsPtrAccessChain(SpvVar variable)
        {
            return spirvOp(5 + variable, 70);
        }

        SpvOp OpDecorate(SpvVar variable)
        {
            return spirvOp(3 + variable, 71);
        }

        SpvOp OpMemberDecorate(SpvVar variable)
        {
            return spirvOp(4 + variable, 72);
        }

        SpvOp OpDecorationGroup()
        {
            return spirvOp(2, 73);
        }

        SpvOp OpGroupDecorate(SpvVar variable)
        {
            return spirvOp(2 + variable, 74);
        }

        SpvOp OpGroupMemberDecorate(SpvVar variable)
        {
            return spirvOp(2 + variable, 75);
        }

        SpvOp OpVectorExtractDynamic()
        {
            return spirvOp(5, 77);
        }

        SpvOp OpVectorInsertDynamic()
        {
            return spirvOp(6, 78);
        }

        SpvOp OpVectorShuffle(SpvVar variable)
        {
            return spirvOp(5 + variable, 79);
        }

        SpvOp OpCompositeConstruct(SpvVar variable)
        {
            return spirvOp(3 + variable, 80);
        }

        SpvOp OpCompositeExtract(SpvVar variable)
        {
            return spirvOp(4 + variable, 81);
        }

        SpvOp OpCompositeInsert(SpvVar variable)
        {
            return spirvOp(5 + variable, 82);
        }

        SpvOp OpCopyObject()
        {
            return spirvOp(4, 83);
        }

        SpvOp OpTranspose()
        {
            return spirvOp(4, 84);
        }

        SpvOp OpSampledImage()
        {
            return spirvOp(5, 86);
        }

        SpvOp OpImageSampleImplicitLod(SpvVar variable)
        {
            return spirvOp(5 + variable, 87);
        }

        SpvOp OpImageSampleExplicitLod(SpvVar variable)
        {
            return spirvOp(7 + variable, 88);
        }

        SpvOp OpImageSampleDrefImplicitLod(SpvVar variable)
        {
            return spirvOp(6 + variable, 89);
        }

        SpvOp OpImageSampleDrefExplicitLod(SpvVar variable)
        {
            return spirvOp(8 + variable, 90);
        }

        SpvOp OpImageSampleProjImplicitLod(SpvVar variable)
        {
            return spirvOp(5 + variable, 91);
        }

        SpvOp OpImageSampleProjExplicitLod(SpvVar variable)
        {
            return spirvOp(7 + variable, 92);
        }

        SpvOp OpImageSampleProjDrefImplicitLod(SpvVar variable)
        {
            return spirvOp(6 + variable, 93);
        }

        SpvOp OpImageSampleProjDrefExplicitLod(SpvVar variable)
        {
            return spirvOp(8 + variable, 94);
        }

        SpvOp OpImageFetch(SpvVar variable)
        {
            return spirvOp(5 + variable, 95);
        }

        SpvOp OpImageGather(SpvVar variable)
        {
            return spirvOp(6 + variable, 96);
        }

        SpvOp OpImageDrefGather(SpvVar variable)
        {
            return spirvOp(6 + variable, 97);
        }

        SpvOp OpImageRead(SpvVar variable)
        {
            return spirvOp(5 + variable, 98);
        }

        SpvOp OpImageWrite(SpvVar variable)
        {
            return spirvOp(4 + variable, 99);
        }

        SpvOp OpImage()
        {
            return spirvOp(4, 100);
        }

        SpvOp OpImageQueryFormat()
        {
            return spirvOp(4, 101);
        }

        SpvOp OpImageQueryOrder()
        {
            return spirvOp(4, 102);
        }

        SpvOp OpImageQuerySizeLod()
        {
            return spirvOp(5, 103);
        }

        SpvOp OpImageQuerySize()
        {
            return spirvOp(4, 104);
        }

        SpvOp OpImageQueryLod()
        {
            return spirvOp(5, 105);
        }

        SpvOp OpImageQueryLevels()
        {
            return spirvOp(4, 106);
        }

        SpvOp OpImageQuerySamples()
        {
            return spirvOp(4, 107);
        }

        SpvOp OpConvertFToU()
        {
            return spirvOp(4, 109);
        }

        SpvOp OpConvertFToS()
        {
            return spirvOp(4, 110);
        }

        SpvOp OpConvertSToF()
        {
            return spirvOp(4, 111);
        }

        SpvOp OpConvertUToF()
        {
            return spirvOp(4, 112);
        }

        SpvOp OpUConvert()
        {
            return spirvOp(4, 113);
        }

        SpvOp OpSConvert()
        {
            return spirvOp(4, 114);
        }

        SpvOp OpFConvert()
        {
            return spirvOp(4, 115);
        }

        SpvOp OpQuantizeToF16()
        {
            return spirvOp(4, 116);
        }

        SpvOp OpConvertPtrToU()
        {
            return spirvOp(4, 117);
        }

        SpvOp OpSatConvertSToU()
        {
            return spirvOp(4, 118);
        }

        SpvOp OpSatConvertUToS()
        {
            return spirvOp(4, 119);
        }

        SpvOp OpConvertUToPtr()
        {
            return spirvOp(4, 120);
        }

        SpvOp OpPtrCastToGeneric()
        {
            return spirvOp(4, 121);
        }

        SpvOp OpGenericCastToPtr()
        {
            return spirvOp(4, 122);
        }

        SpvOp OpGenericCastToPtrExplicit()
        {
            return spirvOp(5, 123);
        }

        SpvOp OpBitcast()
        {
            return spirvOp(4, 124);
        }

        SpvOp OpSNegate()
        {
            return spirvOp(4, 126);
        }

        SpvOp OpFNegate()
        {
            return spirvOp(4, 127);
        }

        SpvOp OpIAdd()
        {
            return spirvOp(5, 128);
        }

        SpvOp OpFAdd()
        {
            return spirvOp(5, 129);
        }

        SpvOp OpISub()
        {
            return spirvOp(5, 130);
        }

        SpvOp OpFSub()
        {
            return spirvOp(5, 131);
        }

        SpvOp OpIMul()
        {
            return spirvOp(5, 132);
        }

        SpvOp OpFMul()
        {
            return spirvOp(5, 133);
        }

        SpvOp OpUDiv()
        {
            return spirvOp(5, 134);
        }

        SpvOp OpSDiv()
        {
            return spirvOp(5, 135);
        }

        SpvOp OpFDiv()
        {
            return spirvOp(5, 136);
        }

        SpvOp OpUMod()
        {
            return spirvOp(5, 137);
        }

        SpvOp OpSRem()
        {
            return spirvOp(5, 138);
        }

        SpvOp OpSMod()
        {
            return spirvOp(5, 139);
        }

        SpvOp OpFRem()
        {
            return spirvOp(5, 140);
        }

        SpvOp OpFMod()
        {
            return spirvOp(5, 141);
        }

        SpvOp OpVectorTimesScalar()
        {
            return spirvOp(5, 142);
        }

        SpvOp OpMatrixTimesScalar()
        {
            return spirvOp(5, 143);
        }

        SpvOp OpVectorTimesMatrix()
        {
            return spirvOp(5, 144);
        }

        SpvOp OpMatrixTimesVector()
        {
            return spirvOp(5, 145);
        }

        SpvOp OpMatrixTimesMatrix()
        {
            return spirvOp(5, 146);
        }

        SpvOp OpOuterProduct()
        {
            return spirvOp(5, 147);
        }

        SpvOp OpDot()
        {
            return spirvOp(5, 148);
        }

        SpvOp OpIAddCarry()
        {
            return spirvOp(5, 149);
        }

        SpvOp OpISubBorrow()
        {
            return spirvOp(5, 150);
        }

        SpvOp OpUMulExtended()
        {
            return spirvOp(5, 151);
        }

        SpvOp OpSMulExtended()
        {
            return spirvOp(5, 152);
        }

        SpvOp OpAny()
        {
            return spirvOp(4, 154);
        }

        SpvOp OpAll()
        {
            return spirvOp(4, 155);
        }

        SpvOp OpIsNan()
        {
            return spirvOp(4, 156);
        }

        SpvOp OpIsInf()
        {
            return spirvOp(4, 157);
        }

        SpvOp OpIsFinite()
        {
            return spirvOp(4, 158);
        }

        SpvOp OpIsNormal()
        {
            return spirvOp(4, 159);
        }

        SpvOp OpSignBitSet()
        {
            return spirvOp(4, 160);
        }

        //Deprecated
        SpvOp OpLessOrGreater()
        {
            return spirvOp(5, 161);
        }

        SpvOp OpOrdered()
        {
            return spirvOp(5, 162);
        }

        SpvOp OpUnordered()
        {
            return spirvOp(5, 163);
        }

        SpvOp OpLogicalEqual()
        {
            return spirvOp(5, 164);
        }

        SpvOp OpLogicalNotEqual()
        {
            return spirvOp(5, 165);
        }

        SpvOp OpLogicalOr()
        {
            return spirvOp(5, 166);
        }

        SpvOp OpLogicalAnd()
        {
            return spirvOp(5, 167);
        }

        SpvOp OpLogicalNot()
        {
            return spirvOp(4, 168);
        }

        SpvOp OpSelect()
        {
            return spirvOp(6, 169);
        }

        SpvOp OpIEqual()
        {
            return spirvOp(5, 170);
        }

        SpvOp OpINotEqual()
        {
            return spirvOp(5, 171);
        }

        SpvOp OpUGreaterThan()
        {
            return spirvOp(5, 172);
        }

        SpvOp OpSGreaterThan()
        {
            return spirvOp(5, 173);
        }

        SpvOp OpUGreaterThanEqual()
        {
            return spirvOp(5, 174);
        }

        SpvOp OpSGreaterThanEqual()
        {
            return spirvOp(5, 175);
        }

        SpvOp OpULessThan()
        {
            return spirvOp(5, 176);
        }

        SpvOp OpSLessThan()
        {
            return spirvOp(5, 177);
        }

        SpvOp OpULessThanEqual()
        {
            return spirvOp(5, 178);
        }

        SpvOp OpSLessThanEqual()
        {
            return spirvOp(5, 179);
        }

        SpvOp OpFOrdEqual()
        {
            return spirvOp(5, 180);
        }

        SpvOp OpFUnordEqual()
        {
            return spirvOp(5, 181);
        }

        SpvOp OpFOrdNotEqual()
        {
            return spirvOp(5, 182);
        }

        SpvOp OpFUnordNotEqual()
        {
            return spirvOp(5, 183);
        }

        SpvOp OpFOrdLessThan()
        {
            return spirvOp(5, 184);
        }

        SpvOp OpFUnordLessThan()
        {
            return spirvOp(5, 185);
        }

        SpvOp OpFOrdGreaterThan()
        {
            return spirvOp(5, 186);
        }

        SpvOp OpFUnordGreaterThan()
        {
            return spirvOp(5, 187);
        }

        SpvOp OpFOrdLessThanEqual()
        {
            return spirvOp(5, 188);
        }

        SpvOp OpFUnordLessThanEqual()
        {
            return spirvOp(5, 189);
        }

        SpvOp OpFOrdGreaterThanEqual()
        {
            return spirvOp(5, 190);
        }

        SpvOp OpFUnordGreaterThanEqual()
        {
            return spirvOp(5, 191);
        }

        SpvOp OpShiftRightLogical()
        {
            return spirvOp(5, 194);
        }

        SpvOp OpShiftRightArithmetic()
        {
            return spirvOp(5, 195);
        }

        SpvOp OpShiftLeftLogical()
        {
            return spirvOp(5, 196);
        }

        SpvOp OpBitwiseOr()
        {
            return spirvOp(5, 197);
        }

        SpvOp OpBitwiseXor()
        {
            return spirvOp(5, 198);
        }

        SpvOp OpBitwiseAnd()
        {
            return spirvOp(5, 199);
        }

        SpvOp OpNot()
        {
            return spirvOp(4, 200);
        }

        SpvOp OpBitFieldInsert()
        {
            return spirvOp(7, 201);
        }

        SpvOp OpBitFieldSExtract()
        {
            return spirvOp(6, 202);
        }

        SpvOp OpBitFieldUExtract()
        {
            return spirvOp(6, 203);
        }

        SpvOp OpBitReverse()
        {
            return spirvOp(4, 204);
        }

        SpvOp OpBitCount()
        {
            return spirvOp(4, 205);
        }

        SpvOp OpDPdx()
        {
            return spirvOp(4, 207);
        }

        SpvOp OpDPdy()
        {
            return spirvOp(4, 208);
        }

        SpvOp OpFwidth()
        {
            return spirvOp(4, 209);
        }

        SpvOp OpDPdxFine()
        {
            return spirvOp(4, 210);
        }

        SpvOp OpDPdyFine()
        {
            return spirvOp(4, 211);
        }

        SpvOp OpFwidthFine()
        {
            return spirvOp(4, 212);
        }

        SpvOp OpDPdxCoarse()
        {
            return spirvOp(4, 213);
        }

        SpvOp OpDPdyCoarse()
        {
            return spirvOp(4, 214);
        }

        SpvOp OpFwidthCoarse()
        {
            return spirvOp(4, 215);
        }

        SpvOp OpEmitVertex()
        {
            return spirvOp(1, 218);
        }

        SpvOp OpEndPrimitive()
        {
            return spirvOp(1, 219);
        }

        SpvOp OpEmitStreamVertex()
        {
            return spirvOp(1, 220);
        }

        SpvOp OpEndStreamPrimitive()
        {
            return spirvOp(2, 221);
        }

        SpvOp OpControlBarrier()
        {
            return spirvOp(4, 224);
        }

        SpvOp OpMemoryBarrier()
        {
            return spirvOp(4, 225);
        }

        SpvOp OpAtomicLoad()
        {
            return spirvOp(6, 227);
        }

        SpvOp OpAtomicStore()
        {
            return spirvOp(5, 228);
        }

        SpvOp OpAtomicExchange()
        {
            return spirvOp(7, 229);
        }

        SpvOp OpAtomicCompareExchange()
        {
            return spirvOp(9, 230);
        }

        SpvOp OpAtomicCompareExchangeWeak()
        {
            return spirvOp(9, 231);
        }

        SpvOp OpAtomicIIncrement()
        {
            return spirvOp(6, 232);
        }

        SpvOp OpAtomicIDecrement()
        {
            return spirvOp(6, 233);
        }

        SpvOp OpAtomicIAdd()
        {
            return spirvOp(7, 234);
        }

        SpvOp OpAtomicISub()
        {
            return spirvOp(7, 235);
        }

        SpvOp OpAtomicSMin()
        {
            return spirvOp(7, 236);
        }

        SpvOp OpAtomicUMin()
        {
            return spirvOp(7, 237);
        }

        SpvOp OpAtomicSMax()
        {
            return spirvOp(7, 238);
        }

        SpvOp OpAtomicUMax()
        {
            return spirvOp(7, 239);
        }

        SpvOp OpAtomicAnd()
        {
            return spirvOp(7, 240);
        }

        SpvOp OpAtomicOr()
        {
            return spirvOp(7, 241);
        }

        SpvOp OpAtomicXor()
        {
            return spirvOp(7, 242);
        }

        SpvOp OpPhi(SpvVar variable)
        {
            return spirvOp(3 + variable, 245);
        }

        SpvOp OpLoopMerge(SpvVar variable)
        {
            return spirvOp(4 + variable, 246);
        }

        SpvOp OpSelectionMerge()
        {
            return spirvOp(3, 247);
        }

        SpvOp OpLabel()
        {
            return spirvOp(2, 248);
        }

        SpvOp OpBranch()
        {
            return spirvOp(2, 249);
        }

        SpvOp OpBranchConditional(SpvVar variable)
        {
            return spirvOp(4 + variable, 250);
        }

        SpvOp OpSwitch(SpvVar variable)
        {
            return spirvOp(3 + variable, 251);
        }

        SpvOp OpKill()
        {
            return spirvOp(1, 252);
        }

        SpvOp OpReturn()
        {
            return spirvOp(1, 253);
        }

        SpvOp OpReturnValue()
        {
            return spirvOp(2, 254);
        }

        SpvOp OpUnreachable()
        {
            return spirvOp(1, 255);
        }

        SpvOp OpLifetimeStart()
        {
            return spirvOp(3, 256);
        }

        SpvOp OpLifetimeStop()
        {
            return spirvOp(3, 257);
        }

        SpvOp OpGroupAsyncCopy()
        {
            return spirvOp(9, 259);
        }

        SpvOp OpGroupWaitEvents()
        {
            return spirvOp(4, 260);
        }

        SpvOp OpGroupAll()
        {
            return spirvOp(5, 261);
        }

        SpvOp OpGroupAny()
        {
            return spirvOp(5, 262);
        }

        SpvOp OpGroupBroadcast()
        {
            return spirvOp(6, 263);
        }

        SpvOp OpGroupIAdd()
        {
            return spirvOp(6, 264);
        }

        SpvOp OpGroupFAdd()
        {
            return spirvOp(6, 265);
        }

        SpvOp OpGroupFMin()
        {
            return spirvOp(6, 266);
        }

        SpvOp OpGroupUMin()
        {
            return spirvOp(6, 267);
        }

        SpvOp OpGroupSMin()
        {
            return spirvOp(6, 268);
        }

        SpvOp OpGroupFMax()
        {
            return spirvOp(6, 269);
        }

        SpvOp OpGroupUMax()
        {
            return spirvOp(6, 270);
        }

        SpvOp OpGroupSMax()
        {
            return spirvOp(6, 271);
        }

        SpvOp OpReadPipe()
        {
            return spirvOp(7, 274);
        }

        SpvOp OpWritePipe()
        {
            return spirvOp(7, 275);
        }

        SpvOp OpReservedReadPipe()
        {
            return spirvOp(9, 276);
        }

        SpvOp OpReservedWritePipe()
        {
            return spirvOp(9, 277);
        }

        SpvOp OpReserveReadPipePackets()
        {
            return spirvOp(7, 278);
        }

        SpvOp OpReserveWritePipePackets()
        {
            return spirvOp(7, 279);
        }

        SpvOp OpCommitReadPipe()
        {
            return spirvOp(5, 280);
        }

        SpvOp OpCommitWritePipe()
        {
            return spirvOp(5, 281);
        }

        SpvOp OpIsValidReserveId()
        {
            return spirvOp(4, 282);
        }

        SpvOp OpGetNumPipePackets()
        {
            return spirvOp(5, 283);
        }

        SpvOp OpGetMaxPipePackets()
        {
            return spirvOp(6, 284);
        }

        SpvOp OpGroupReserveReadPipePackets()
        {
            return spirvOp(8, 285);
        }

        SpvOp OpGroupReserveWritePipePackets()
        {
            return spirvOp(8, 286);
        }

        SpvOp OpGroupCommitReadPipe()
        {
            return spirvOp(6, 287);
        }

        SpvOp OpGroupCommitWritePipe()
        {
            return spirvOp(6, 288);
        }

        SpvOp OpEnqueueMarker()
        {
            return spirvOp(7, 291);
        }

        SpvOp OpEnqueueKernel(SpvVar variable)
        {
            return spirvOp(13 + variable, 292);
        }

        SpvOp OpGetKernelNDrangeSubGroupCount()
        {
            return spirvOp(8, 293);
        }

        SpvOp OpGetKernelNDrangeMaxSubGroupSize()
        {
            return spirvOp(8, 294);
        }

        SpvOp OpGetKernelWorkGroupSize()
        {
            return spirvOp(7, 295);
        }

        SpvOp OpGetKernelPreferredWorkGroupSizeMultiple()
        {
            return spirvOp(7, 296);
        }

        SpvOp OpRetainEvent()
        {
            return spirvOp(2, 297);
        }

        SpvOp OpReleaseEvent()
        {
            return spirvOp(2, 298);
        }

        SpvOp OpCreateUserEvent()
        {
            return spirvOp(3, 299);
        }

        SpvOp OpIsValidEvent()
        {
            return spirvOp(4, 300);
        }

        SpvOp OpSetUserEventStatus()
        {
            return spirvOp(3, 301);
        }

        SpvOp OpCaptureEventProfilingInfo()
        {
            return spirvOp(4, 302);
        }

        SpvOp OpGetDefaultQueue()
        {
            return spirvOp(3, 303);
        }

        SpvOp OpBuildNDRange()
        {
            return spirvOp(6, 304);
        }

        SpvOp OpImageQuerySamples(SpvVar variable)
        {
            return spirvOp(5 + variable, 305);
        }

        SpvOp OpImageSparseSampleExplicitLod(SpvVar variable)
        {
            return spirvOp(7 + variable, 306);
        }

        SpvOp OpImageSparseSampleDrefImplicitLod(SpvVar variable)
        {
            return spirvOp(6 + variable, 307);
        }

        SpvOp OpImageSparseSampleDrefExplicitLod(SpvVar variable)
        {
            return spirvOp(8 + variable, 308);
        }

        SpvOp OpImageSparseSampleProjImplicitLod(SpvVar variable)
        {
            return spirvOp(5 + variable, 309);
        }

        SpvOp OpImageSparseSampleProjExplicitLod(SpvVar variable)
        {
            return spirvOp(7 + variable, 310);
        }

        SpvOp OpImageSparseSampleProjDrefImplicitLod(SpvVar variable)
        {
            return spirvOp(6 + variable, 311);
        }

        SpvOp OpImageSparseFetch(SpvVar variable)
        {
            return spirvOp(5 + variable, 313);
        }

        SpvOp OpImageSparseGather(SpvVar variable)
        {
            return spirvOp(6 + variable, 314);
        }

        SpvOp OpImageSparseDrefGather(SpvVar variable)
        {
            return spirvOp(6 + variable, 315);
        }

        SpvOp OpImageSparseDrefGather()
        {
            return spirvOp(4, 316);
        }

        SpvOp OpNoLine()
        {
            return spirvOp(1, 317);
        }

        SpvOp OpAtomicFlagTestAndSet()
        {
            return spirvOp(6, 318);
        }

        SpvOp OpAtomicFlagClear()
        {
            return spirvOp(4, 319);
        }

        SpvOp OpImageSparseRead(SpvVar variable)
        {
            return spirvOp(5 + variable, 320);
        }

        //Version >= 1.1
        SpvOp OpSizeOf()
        {
            return spirvOp(4, 321);
        }

        //Version >= 1.1
        SpvOp OpTypePipeStorage()
        {
            return spirvOp(3, 322);
        }

        //Version >= 1.1
        SpvOp OpConstantPipeStorage()
        {
            return spirvOp(6, 323);
        }

        //Version >= 1.1
        SpvOp OpCreatePipeFromPipeStorage()
        {
            return spirvOp(4, 324);
        }

        //Version >= 1.1
        SpvOp OpGetKernelLocalSizeForSubgroupCount()
        {
            return spirvOp(8, 325);
        }

        //Version >= 1.1
        SpvOp OpGetKernelMaxNumSubgroups()
        {
            return spirvOp(7, 326);
        }

        //Version >= 1.1
        SpvOp OpTypeNamedBarrier()
        {
            return spirvOp(3, 327);
        }

        //Version >= 1.1
        SpvOp OpNamedBarrierInitialize()
        {
            return spirvOp(4, 328);
        }

        //Version >= 1.1
        SpvOp OpMemoryNamedBarrier()
        {
            return spirvOp(4, 329);
        }

        //Version >= 1.1
        SpvOp OpModuleProcessed(SpvVar variable)
        {
            return spirvOp(2 + variable, 330);
        }

        //Version >= 1.2
        SpvOp OpExecutionModeId(SpvVar variable)
        {
            return spirvOp(3 + variable, 331);
        }

        //Version >= 1.2
        SpvOp OpDecorateId(SpvVar variable)
        {
            return spirvOp(3 + variable, 332);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformElect()
        {
            return spirvOp(4, 333);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformAll()
        {
            return spirvOp(5, 334);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformAny()
        {
            return spirvOp(5, 335);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformAllEqual()
        {
            return spirvOp(5, 336);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBroadcast()
        {
            return spirvOp(6, 337);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBroadcastFirst()
        {
            return spirvOp(5, 338);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBallot()
        {
            return spirvOp(5, 339);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformInverseBallot()
        {
            return spirvOp(5, 340);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBallotBitExtract()
        {
            return spirvOp(6, 341);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBallotBitCount()
        {
            return spirvOp(6, 342);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBallotFindLSB()
        {
            return spirvOp(5, 343);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBallotFindMSB()
        {
            return spirvOp(5, 344);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformShuffle()
        {
            return spirvOp(6, 345);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformShuffleXor()
        {
            return spirvOp(6, 346);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformShuffleUp()
        {
            return spirvOp(6, 347);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformShuffleDown()
        {
            return spirvOp(6, 348);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformIAdd(SpvVar variable)
        {
            return spirvOp(6 + variable, 349);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformFAdd(SpvVar variable)
        {
            return spirvOp(6 + variable, 350);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformIMul(SpvVar variable)
        {
            return spirvOp(6 + variable, 351);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformFMul(SpvVar variable)
        {
            return spirvOp(6 + variable, 352);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformSMin(SpvVar variable)
        {
            return spirvOp(6 + variable, 353);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformUMin(SpvVar variable)
        {
            return spirvOp(6 + variable, 354);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformFMin(SpvVar variable)
        {
            return spirvOp(6 + variable, 355);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformSMax(SpvVar variable)
        {
            return spirvOp(6 + variable, 356);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformUMax(SpvVar variable)
        {
            return spirvOp(6 + variable, 357);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformFMax(SpvVar variable)
        {
            return spirvOp(6 + variable, 358);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBitwiseAnd(SpvVar variable)
        {
            return spirvOp(6 + variable, 359);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBitwiseOr(SpvVar variable)
        {
            return spirvOp(6 + variable, 360);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformBitwiseXor(SpvVar variable)
        {
            return spirvOp(6 + variable, 361);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformLogicalAnd(SpvVar variable)
        {
            return spirvOp(6 + variable, 362);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformLogicalOr(SpvVar variable)
        {
            return spirvOp(6 + variable, 363);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformLogicalXor(SpvVar variable)
        {
            return spirvOp(6 + variable, 364);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformQuadBroadcast()
        {
            return spirvOp(6, 365);
        }

        //Version >= 1.3
        SpvOp OpGroupNonUniformQuadSwap()
        {
            return spirvOp(6, 366);
        }

        SpvOp OpCopyLogical()
        {
            return spirvOp(4, 400);
        }

        //Version >= 1.4
        SpvOp OpPtrEqual()
        {
            return spirvOp(5, 401);
        }

        //Version >= 1.4
        SpvOp OpPtrNotEqual()
        {
            return spirvOp(5, 402);
        }

        //Version >= 1.4
        SpvOp OpPtrDiff()
        {
            return spirvOp(5, 403);
        }

        //Version >= 1.2
        SpvOp OpMemberDecorateString(SpvVar variable)
        {
            return spirvOp(5 + variable, 5633);
        }

    }
	
}
