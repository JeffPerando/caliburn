
#pragma once

#include <map>
#include <set>
#include <vector>

#include "basic.h"
#include "cllrasm.h"
#include "cllrout.h"
#include "spirvasm.h"

namespace caliburn
{
	namespace cllr
	{
		class SPIRVOutAssembler;

		//Function pointer type for easier usage later
		using SPIRVOutFn = void(Target target, ref<cllr::Instruction> i, size_t off, ref<cllr::Assembler> in, ref<SPIRVOutAssembler> out);
		using SPIRVOpList = std::initializer_list<spirv::SpvOp>;

		//Macro shorthand for implementation signature
		#define CLLR_SPIRV_IMPL(Name) void Name(Target target, ref<Instruction> i, size_t off, ref<cllr::Assembler> in, ref<SPIRVOutAssembler> out)
		
		//I hate how wordy modern C++ can be...
		#define SPIRV_CODE_SECTION(...) new_uptr<spirv::CodeSection>(__VA_ARGS__)

		namespace spirv_impl
		{
			CLLR_SPIRV_IMPL(OpUnknown);

			CLLR_SPIRV_IMPL(OpKernel);
			CLLR_SPIRV_IMPL(OpKernelEnd);

			CLLR_SPIRV_IMPL(OpFunction);
			CLLR_SPIRV_IMPL(OpFunctionEnd);

			CLLR_SPIRV_IMPL(OpVarLocal);
			CLLR_SPIRV_IMPL(OpVarGlobal);
			CLLR_SPIRV_IMPL(OpVarFuncArg);
			CLLR_SPIRV_IMPL(OpVarShaderIn);
			CLLR_SPIRV_IMPL(OpVarShaderOut);
			CLLR_SPIRV_IMPL(OpVarDescriptor);

			CLLR_SPIRV_IMPL(OpCall);
			//CLLR_SPIRV_IMPL(OpDispatch);
			CLLR_SPIRV_IMPL(OpCallArg);

			CLLR_SPIRV_IMPL(OpTypeVoid);
			CLLR_SPIRV_IMPL(OpTypeFloat);
			CLLR_SPIRV_IMPL(OpTypeInt);
			CLLR_SPIRV_IMPL(OpTypeArray);
			CLLR_SPIRV_IMPL(OpTypeVector);
			CLLR_SPIRV_IMPL(OpTypeMatrix);
			CLLR_SPIRV_IMPL(OpTypeStruct);

			CLLR_SPIRV_IMPL(OpTypeBool);
			CLLR_SPIRV_IMPL(OpTypePtr);
			CLLR_SPIRV_IMPL(OpTypeTuple);
			CLLR_SPIRV_IMPL(OpTypeString);

			CLLR_SPIRV_IMPL(OpStructMember);
			CLLR_SPIRV_IMPL(OpStructEnd);

			CLLR_SPIRV_IMPL(OpLabel);
			CLLR_SPIRV_IMPL(OpJump);
			CLLR_SPIRV_IMPL(OpJumpCond);
			CLLR_SPIRV_IMPL(OpLoop);

			CLLR_SPIRV_IMPL(OpAssign);
			CLLR_SPIRV_IMPL(OpCompare);

			CLLR_SPIRV_IMPL(OpValueArrayLit);
			CLLR_SPIRV_IMPL(OpValueBoolLit);
			CLLR_SPIRV_IMPL(OpValueCast);
			CLLR_SPIRV_IMPL(OpValueDeref);
			CLLR_SPIRV_IMPL(OpValueExpr);
			CLLR_SPIRV_IMPL(OpValueExprUnary);
			CLLR_SPIRV_IMPL(OpValueFloatLit);
			CLLR_SPIRV_IMPL(OpValueIntLit);
			CLLR_SPIRV_IMPL(OpValueInvokePos);
			CLLR_SPIRV_IMPL(OpValueInvokeSize);
			CLLR_SPIRV_IMPL(OpValueMember);
			CLLR_SPIRV_IMPL(OpValueNull);
			CLLR_SPIRV_IMPL(OpValueReadVar);
			CLLR_SPIRV_IMPL(OpValueStrLit);
			CLLR_SPIRV_IMPL(OpValueSubarray);
			CLLR_SPIRV_IMPL(OpValueZero);

			CLLR_SPIRV_IMPL(OpReturn);
			CLLR_SPIRV_IMPL(OpReturnValue);
			CLLR_SPIRV_IMPL(OpDiscard);

		}
		
		class SPIRVOutAssembler : cllr::OutAssembler<uint32_t>
		{
			const uptr<spirv::CodeSection> spvHeader = SPIRV_CODE_SECTION(this, SPIRVOpList{
				spirv::OpCapability(),
				spirv::OpExtension()
			});
			const uptr<spirv::CodeSection> spvImports = SPIRV_CODE_SECTION(this, SPIRVOpList{
				spirv::OpExtInstImport()
			});
			const uptr<spirv::CodeSection> spvMisc = SPIRV_CODE_SECTION(this, SPIRVOpList{
				spirv::OpMemoryModel(),
				spirv::OpExecutionMode(),
				spirv::OpExecutionModeId()
			});
			const uptr<spirv::CodeSection> spvDebug = SPIRV_CODE_SECTION(this, SPIRVOpList{
				spirv::OpString(),
				spirv::OpSource(),
				spirv::OpSourceExtension(),
				spirv::OpSourceContinued(),
				spirv::OpName(),
				spirv::OpMemberName(),
				spirv::OpModuleProcessed()
			});
			const uptr<spirv::CodeSection> spvGloVars = SPIRV_CODE_SECTION(this, SPIRVOpList{
				spirv::OpVariable()
			});

			uint32_t ssa = 1;
			HashMap<cllr::SSA, spirv::SSA> ssaAliases;

			std::vector<spirv::SSAEntry> ssaEntries;
			std::set<spirv::Capability> capabilities;
			std::vector<std::string> extensions;
			std::vector<std::string> instructions{"GLSL.std.450"};//TODO reconsider

			spirv::AddressingModel addrModel = spirv::AddressingModel::Logical;
			spirv::MemoryModel memModel = spirv::MemoryModel::GLSL450;

			OutImpls<SPIRVOutFn> impls = {};

		public:
			std::vector<spirv::EntryPoint> shaderEntries;

			const uptr<spirv::CodeSection> decs = SPIRV_CODE_SECTION(this, SPIRVOpList{
				spirv::OpDecorate(),
				spirv::OpGroupDecorate(),
				spirv::OpGroupMemberDecorate(),
				spirv::OpMemberDecorate(),
				spirv::OpMemberDecorateString(),
				spirv::OpDecorationGroup()
			});
			const uptr<spirv::CodeSection> types = SPIRV_CODE_SECTION(this, SPIRVOpList{
				spirv::OpTypeArray(),
				spirv::OpTypeBool(),
				spirv::OpTypeFloat(),
				spirv::OpTypeFunction(),
				spirv::OpTypeImage(),
				spirv::OpTypeInt(),
				spirv::OpTypeMatrix(),
				spirv::OpTypeOpaque(),
				spirv::OpTypePointer(),
				spirv::OpTypeSampler(),
				spirv::OpTypeStruct(),
				spirv::OpTypeVector(),
				spirv::OpTypeVoid(),
				spirv::OpLine(),
				spirv::OpNoLine()
			});
			const uptr<spirv::CodeSection> consts = SPIRV_CODE_SECTION(this, SPIRVOpList{
				spirv::OpConstant(),
				spirv::OpConstantComposite(),
				spirv::OpConstantFalse(),
				spirv::OpConstantNull(),
				spirv::OpConstantSampler(),
				spirv::OpConstantTrue(),
				spirv::OpSpecConstantComposite(),
				spirv::OpSpecConstantFalse(),
				spirv::OpSpecConstantOp(),
				spirv::OpSpecConstantTrue(),
				spirv::OpLine(),
				spirv::OpNoLine()
			});
			const uptr<spirv::CodeSection> main = SPIRV_CODE_SECTION(this, SPIRVOpList{});

			SPIRVOutAssembler() : OutAssembler(Target::GPU)
			{
				//here we go...
				
				impls[(uint32_t)Opcode::KERNEL] = spirv_impl::OpKernel;
				impls[(uint32_t)Opcode::KERNEL_END] = spirv_impl::OpKernelEnd;

				impls[(uint32_t)Opcode::FUNCTION] = spirv_impl::OpFunction;
				impls[(uint32_t)Opcode::FUNCTION_END] = spirv_impl::OpFunctionEnd;

				impls[(uint32_t)Opcode::VAR_LOCAL] = spirv_impl::OpVarLocal;
				impls[(uint32_t)Opcode::VAR_GLOBAL] = spirv_impl::OpVarGlobal;
				impls[(uint32_t)Opcode::VAR_FUNC_ARG] = spirv_impl::OpVarFuncArg;
				impls[(uint32_t)Opcode::VAR_SHADER_IN] = spirv_impl::OpVarShaderIn;
				impls[(uint32_t)Opcode::VAR_SHADER_OUT] = spirv_impl::OpVarShaderOut;
				impls[(uint32_t)Opcode::VAR_DESCRIPTOR] = spirv_impl::OpVarDescriptor;

				impls[(uint32_t)Opcode::CALL] = spirv_impl::OpCall;
				impls[(uint32_t)Opcode::CALL_ARG] = spirv_impl::OpCallArg;

				impls[(uint32_t)Opcode::TYPE_VOID] = spirv_impl::OpTypeVoid;
				impls[(uint32_t)Opcode::TYPE_FLOAT] = spirv_impl::OpTypeFloat;
				impls[(uint32_t)Opcode::TYPE_INT] = spirv_impl::OpTypeInt;
				impls[(uint32_t)Opcode::TYPE_ARRAY] = spirv_impl::OpTypeArray;
				impls[(uint32_t)Opcode::TYPE_VECTOR] = spirv_impl::OpTypeVector;
				impls[(uint32_t)Opcode::TYPE_MATRIX] = spirv_impl::OpTypeMatrix;

				impls[(uint32_t)Opcode::TYPE_STRUCT] = spirv_impl::OpTypeStruct;
				impls[(uint32_t)Opcode::STRUCT_MEMBER] = spirv_impl::OpStructMember;
				impls[(uint32_t)Opcode::STRUCT_END] = spirv_impl::OpStructEnd;

				impls[(uint32_t)Opcode::TYPE_BOOL] = spirv_impl::OpTypeBool;
				impls[(uint32_t)Opcode::TYPE_PTR] = spirv_impl::OpTypePtr;
				impls[(uint32_t)Opcode::TYPE_TUPLE] = spirv_impl::OpTypeTuple;
				impls[(uint32_t)Opcode::TYPE_STRING] = spirv_impl::OpTypeString;

				impls[(uint32_t)Opcode::LABEL] = spirv_impl::OpLabel;
				impls[(uint32_t)Opcode::JUMP] = spirv_impl::OpJump;
				impls[(uint32_t)Opcode::JUMP_COND] = spirv_impl::OpJumpCond;
				impls[(uint32_t)Opcode::LOOP] = spirv_impl::OpLoop;

				impls[(uint32_t)Opcode::ASSIGN] = spirv_impl::OpAssign;
				impls[(uint32_t)Opcode::COMPARE] = spirv_impl::OpCompare;

				impls[(uint32_t)Opcode::VALUE_CAST] = spirv_impl::OpValueCast;
				impls[(uint32_t)Opcode::VALUE_DEREF] = spirv_impl::OpValueDeref;
				impls[(uint32_t)Opcode::VALUE_EXPR] = spirv_impl::OpValueExpr;
				impls[(uint32_t)Opcode::VALUE_EXPR_UNARY] = spirv_impl::OpValueExprUnary;
				impls[(uint32_t)Opcode::VALUE_INVOKE_POS] = spirv_impl::OpValueInvokePos;
				impls[(uint32_t)Opcode::VALUE_INVOKE_SIZE] = spirv_impl::OpValueInvokeSize;
				impls[(uint32_t)Opcode::VALUE_LIT_ARRAY] = spirv_impl::OpValueArrayLit;
				impls[(uint32_t)Opcode::VALUE_LIT_BOOL] = spirv_impl::OpValueBoolLit;
				impls[(uint32_t)Opcode::VALUE_LIT_FP] = spirv_impl::OpValueFloatLit;
				impls[(uint32_t)Opcode::VALUE_LIT_INT] = spirv_impl::OpValueIntLit;
				impls[(uint32_t)Opcode::VALUE_LIT_STR] = spirv_impl::OpValueStrLit;
				impls[(uint32_t)Opcode::VALUE_MEMBER] = spirv_impl::OpValueMember;
				impls[(uint32_t)Opcode::VALUE_NULL] = spirv_impl::OpValueNull;
				impls[(uint32_t)Opcode::VALUE_READ_VAR] = spirv_impl::OpValueReadVar;
				impls[(uint32_t)Opcode::VALUE_SUBARRAY] = spirv_impl::OpValueSubarray;
				impls[(uint32_t)Opcode::VALUE_ZERO] = spirv_impl::OpValueZero;

				impls[(uint32_t)Opcode::RETURN] = spirv_impl::OpReturn;
				impls[(uint32_t)Opcode::RETURN_VALUE] = spirv_impl::OpReturnValue;
				impls[(uint32_t)Opcode::DISCARD] = spirv_impl::OpDiscard;

			}

			virtual ~SPIRVOutAssembler() {}

		private:
			constexpr uint32_t maxSSA() const
			{
				return ssa + 1;
			}

		public:
			uptr<std::vector<spirv::SSA>> translateCLLR(ref<cllr::Assembler> cllrAsm, ref<std::vector<sptr<cllr::Instruction>>> code) override;

			spirv::SSA createSSA();

			spirv::SSA toSpvID(cllr::SSA ssa);

			void setOpForSSA(spirv::SSA id, spirv::SpvOp op);

			void addExt(std::string ext);

			SSA addImport(std::string instructions);

			SSA addGlobalVar(SSA type, spirv::StorageClass stClass, SSA init);

			void setMemoryModel(spirv::AddressingModel addr, spirv::MemoryModel mem);

		};

	}

}
