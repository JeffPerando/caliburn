
#pragma once

#include <map>
#include <vector>

#include "basic.h"
#include "cllrout.h"
#include "spirvasm.h"

namespace caliburn
{
	namespace cllr
	{
		class SPIRVOutAssembler;

		//Function pointer type for easier usage later
		using SPIRVOutFn = void(Target target, ref<cllr::Instruction> i, ref<cllr::Assembler> in, ref<SPIRVOutAssembler> out);

		//Macro shorthand for implementation signature
		#define CLLR_SPIRV_IMPL(Name) void Name(Target target, ref<Instruction> i, ref<cllr::Assembler> in, ref<SPIRVOutAssembler> out)

		namespace spirv_impl
		{
			CLLR_SPIRV_IMPL(OpUnknown);

			CLLR_SPIRV_IMPL(OpShaderStage);
			CLLR_SPIRV_IMPL(OpDescriptor);
			CLLR_SPIRV_IMPL(OpShaderEnd);

			CLLR_SPIRV_IMPL(OpFunction);
			CLLR_SPIRV_IMPL(OpFunctionEnd);

			CLLR_SPIRV_IMPL(OpVarLocal);
			CLLR_SPIRV_IMPL(OpVarGlobal);
			CLLR_SPIRV_IMPL(OpVarFuncArg);
			CLLR_SPIRV_IMPL(OpVarShaderIn);
			CLLR_SPIRV_IMPL(OpVarShaderOut);

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
			CLLR_SPIRV_IMPL(OpValueDescriptor);
			CLLR_SPIRV_IMPL(OpValueExpr);
			CLLR_SPIRV_IMPL(OpValueExprUnary);
			CLLR_SPIRV_IMPL(OpValueFloatLit);
			CLLR_SPIRV_IMPL(OpValueInit);
			CLLR_SPIRV_IMPL(OpValueInstanceof);
			CLLR_SPIRV_IMPL(OpValueIntLit);
			CLLR_SPIRV_IMPL(OpValueInvokePos);
			CLLR_SPIRV_IMPL(OpValueInvokeSize);
			CLLR_SPIRV_IMPL(OpValueMember);
			CLLR_SPIRV_IMPL(OpValueNull);
			CLLR_SPIRV_IMPL(OpValueStrLit);
			CLLR_SPIRV_IMPL(OpValueSubarray);
			CLLR_SPIRV_IMPL(OpValueVariable);

			CLLR_SPIRV_IMPL(OpReturn);
			CLLR_SPIRV_IMPL(OpReturnValue);
			CLLR_SPIRV_IMPL(OpDiscard);

		}
		
		class SPIRVOutAssembler : cllr::OutAssembler<uint32_t>
		{
			const sptr<spirv::CodeSection> spvHeader, spvImports, spvMisc, spvDebug, spvGloVars;

			uint32_t ssa = 1;
			std::vector<spirv::SSAEntry> ssaEntries;
			std::map<cllr::SSA, spirv::SSA> ssaAliases;

			std::vector<spirv::Capability> capabilities;
			std::vector<std::string> extensions, instructions;
			spirv::AddressingModel addrModel = spirv::AddressingModel::Logical;
			spirv::MemoryModel memModel = spirv::MemoryModel::GLSL450;
			std::vector<spirv::EntryPoint> entries;
			
			OutImpls<SPIRVOutFn> impls = {};

		public:
			const sptr<spirv::CodeSection> types, decs, consts, main;

			SPIRVOutAssembler() : OutAssembler(Target::GPU)
			{
				//here we go...

				impls[(uint32_t)Opcode::SHADER_STAGE] = spirv_impl::OpShaderStage;
				impls[(uint32_t)Opcode::DESCRIPTOR] = spirv_impl::OpDescriptor;
				impls[(uint32_t)Opcode::SHADER_END] = spirv_impl::OpShaderEnd;

				impls[(uint32_t)Opcode::FUNCTION] = spirv_impl::OpFunction;
				impls[(uint32_t)Opcode::FUNCTION_END] = spirv_impl::OpFunctionEnd;

				impls[(uint32_t)Opcode::VAR_LOCAL] = spirv_impl::OpVarLocal;
				impls[(uint32_t)Opcode::VAR_GLOBAL] = spirv_impl::OpVarGlobal;
				impls[(uint32_t)Opcode::VAR_FUNC_ARG] = spirv_impl::OpVarFuncArg;
				impls[(uint32_t)Opcode::VAR_SHADER_IN] = spirv_impl::OpVarShaderIn;
				impls[(uint32_t)Opcode::VAR_SHADER_OUT] = spirv_impl::OpVarShaderOut;

				impls[(uint32_t)Opcode::CALL] = spirv_impl::OpCall;
				impls[(uint32_t)Opcode::CALL_ARG] = spirv_impl::OpCallArg;

				impls[(uint32_t)Opcode::TYPE_VOID] = spirv_impl::OpTypeVoid;
				impls[(uint32_t)Opcode::TYPE_FLOAT] = spirv_impl::OpTypeFloat;
				impls[(uint32_t)Opcode::TYPE_INT] = spirv_impl::OpTypeInt;
				impls[(uint32_t)Opcode::TYPE_ARRAY] = spirv_impl::OpTypeArray;
				impls[(uint32_t)Opcode::TYPE_VECTOR] = spirv_impl::OpTypeVector;
				impls[(uint32_t)Opcode::TYPE_MATRIX] = spirv_impl::OpTypeMatrix;
				impls[(uint32_t)Opcode::TYPE_STRUCT] = spirv_impl::OpTypeStruct;

				impls[(uint32_t)Opcode::TYPE_BOOL] = spirv_impl::OpTypeBool;
				impls[(uint32_t)Opcode::TYPE_PTR] = spirv_impl::OpTypePtr;
				impls[(uint32_t)Opcode::TYPE_TUPLE] = spirv_impl::OpTypeTuple;
				impls[(uint32_t)Opcode::TYPE_STRING] = spirv_impl::OpTypeString;

				impls[(uint32_t)Opcode::STRUCT_MEMBER] = spirv_impl::OpStructMember;
				impls[(uint32_t)Opcode::STRUCT_END] = spirv_impl::OpStructEnd;

				impls[(uint32_t)Opcode::LABEL] = spirv_impl::OpLabel;
				impls[(uint32_t)Opcode::JUMP] = spirv_impl::OpJump;
				impls[(uint32_t)Opcode::JUMP_COND] = spirv_impl::OpJumpCond;
				impls[(uint32_t)Opcode::LOOP] = spirv_impl::OpLoop;

				impls[(uint32_t)Opcode::ASSIGN] = spirv_impl::OpAssign;
				impls[(uint32_t)Opcode::COMPARE] = spirv_impl::OpCompare;

				impls[(uint32_t)Opcode::VALUE_ARRAY_LIT] = spirv_impl::OpValueArrayLit;
				impls[(uint32_t)Opcode::VALUE_BOOL_LIT] = spirv_impl::OpValueBoolLit;
				impls[(uint32_t)Opcode::VALUE_CAST] = spirv_impl::OpValueCast;
				impls[(uint32_t)Opcode::VALUE_DEREF] = spirv_impl::OpValueDeref;
				impls[(uint32_t)Opcode::VALUE_DESCRIPTOR] = spirv_impl::OpValueDescriptor;
				impls[(uint32_t)Opcode::VALUE_EXPR] = spirv_impl::OpValueExpr;
				impls[(uint32_t)Opcode::VALUE_EXPR_UNARY] = spirv_impl::OpValueExprUnary;
				impls[(uint32_t)Opcode::VALUE_FP_LIT] = spirv_impl::OpValueFloatLit;
				impls[(uint32_t)Opcode::VALUE_INIT] = spirv_impl::OpValueInit;
				impls[(uint32_t)Opcode::VALUE_INSTANCEOF] = spirv_impl::OpValueInstanceof;
				impls[(uint32_t)Opcode::VALUE_INT_LIT] = spirv_impl::OpValueIntLit;
				impls[(uint32_t)Opcode::VALUE_INVOKE_POS] = spirv_impl::OpValueInvokePos;
				impls[(uint32_t)Opcode::VALUE_INVOKE_SIZE] = spirv_impl::OpValueInvokeSize;
				impls[(uint32_t)Opcode::VALUE_MEMBER] = spirv_impl::OpValueMember;
				impls[(uint32_t)Opcode::VALUE_NULL] = spirv_impl::OpValueNull;
				impls[(uint32_t)Opcode::VALUE_STR_LIT] = spirv_impl::OpValueStrLit;
				impls[(uint32_t)Opcode::VALUE_SUBARRAY] = spirv_impl::OpValueSubarray;
				impls[(uint32_t)Opcode::VALUE_VARIABLE] = spirv_impl::OpValueVariable;
				
				impls[(uint32_t)Opcode::RETURN] = spirv_impl::OpReturn;
				impls[(uint32_t)Opcode::RETURN_VALUE] = spirv_impl::OpReturnValue;
				impls[(uint32_t)Opcode::DISCARD] = spirv_impl::OpDiscard;

				/* Something is broken in here
				auto me = std::make_shared<Assembler>(this);
				spvImports = std::make_shared<CodeSection>(me, std::initializer_list<SpvOp>{
					OpExtInstImport()
				});
				spvTypes = std::make_shared<CodeSection>(me, std::initializer_list<SpvOp>{
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
				});
				spvDecs = std::make_shared<CodeSection>(me, std::initializer_list<SpvOp>{
					OpDecorate(),
					OpGroupDecorate(),
					OpGroupMemberDecorate(),
					OpMemberDecorate(),
					OpMemberDecorateString(),
					OpDecorationGroup()
				});
				spvConsts = std::make_shared<CodeSection>(me, std::initializer_list<SpvOp>{
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
				});
				spvGloVars = std::make_shared<CodeSection>(me, std::initializer_list<SpvOp>{
					OpVariable()
				});
				spvCode = std::make_shared<CodeSection>(me, std::initializer_list<SpvOp>{});
				*/
			}

			virtual ~SPIRVOutAssembler() {}

		private:
			uint32_t maxSSA()
			{
				return ssa + 1;
			}

		public:
			uptr<std::vector<spirv::SSA>> translateCLLR(ref<cllr::Assembler> cllrAsm, ref<std::vector<sptr<cllr::Instruction>>> code) override;

			SSA createSSA(spirv::SpvOp op);

			spirv::SSA getOrCreateAlias(cllr::SSA ssa, spirv::SpvOp op);

			void addExt(std::string ext);

			//popular import is "GLSL.std.450"
			SSA addImport(std::string instructions);

			SSA addGlobalVar(SSA type, spirv::StorageClass stClass, SSA init);

			void addEntryPoint(SSA fn, spirv::ExecutionModel type, std::initializer_list<uint32_t> ios);

		};

	}

}
