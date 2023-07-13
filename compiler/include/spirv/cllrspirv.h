
#pragma once

#include <functional>
#include <map>
#include <set>
#include <vector>

#include "basic.h"
#include "cllr/cllrasm.h"
#include "cllr/cllrout.h"
#include "spirv/spirvasm.h"
#include "spirv/spirvio.h"

namespace caliburn
{
	namespace cllr
	{
		class SPIRVOutAssembler;

		//Function pointer type for easier usage later
		using SPIRVOutFn = void(ref<cllr::Instruction> i, size_t off, ref<cllr::Assembler> in, ref<SPIRVOutAssembler> out);
		using SPIRVOpList = std::initializer_list<spirv::SpvOp>;

		//Macro shorthand for implementation signature
		#define CLLR_SPIRV_IMPL(Name) void Name(ref<Instruction> i, size_t off, ref<cllr::Assembler> in, ref<SPIRVOutAssembler> out)
		
		//I hate how wordy modern C++ can be...
		#define SPIRV_CODE_SECTION(...) new_uptr<spirv::CodeSection>(__VA_ARGS__)

		namespace spirv_impl
		{
			static const HashMap<ShaderType, spirv::ExecutionModel> SHADER_EXECUTION_MODELS = {
				{ShaderType::COMPUTE, spirv::ExecutionModel::GLCompute},
				{ShaderType::VERTEX, spirv::ExecutionModel::Vertex},
				{ShaderType::FRAGMENT, spirv::ExecutionModel::Fragment},
				{ShaderType::TESS_CTRL, spirv::ExecutionModel::TessellationControl},
				{ShaderType::TESS_EVAL, spirv::ExecutionModel::TessellationEvaluation},
				{ShaderType::GEOMETRY, spirv::ExecutionModel::Geometry},
				{ShaderType::RT_GEN, spirv::ExecutionModel::RayGenerationKHR},
				{ShaderType::RT_CLOSE, spirv::ExecutionModel::ClosestHitKHR},
				{ShaderType::RT_ANY_HIT, spirv::ExecutionModel::AnyHitKHR},
				{ShaderType::RT_INTERSECT, spirv::ExecutionModel::IntersectionKHR},
				{ShaderType::RT_MISS, spirv::ExecutionModel::MissKHR},
				{ShaderType::TASK, spirv::ExecutionModel::TaskEXT},
				{ShaderType::MESH, spirv::ExecutionModel::MeshEXT}
			};

			CLLR_SPIRV_IMPL(OpUnknown);

			CLLR_SPIRV_IMPL(OpShaderStage);
			CLLR_SPIRV_IMPL(OpShaderStageEnd);
			/*
			CLLR_SPIRV_IMPL(OpKernel);
			CLLR_SPIRV_IMPL(OpKernelEnd);
			*/
			CLLR_SPIRV_IMPL(OpFunction);
			CLLR_SPIRV_IMPL(OpVarFuncArg);
			CLLR_SPIRV_IMPL(OpFunctionEnd);

			CLLR_SPIRV_IMPL(OpVarLocal);
			CLLR_SPIRV_IMPL(OpVarGlobal);
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
			//CLLR_SPIRV_IMPL(OpTypeString);

			CLLR_SPIRV_IMPL(OpStructMember);
			CLLR_SPIRV_IMPL(OpStructEnd);

			CLLR_SPIRV_IMPL(OpLabel);
			CLLR_SPIRV_IMPL(OpJump);
			CLLR_SPIRV_IMPL(OpJumpCond);
			CLLR_SPIRV_IMPL(OpLoop);

			CLLR_SPIRV_IMPL(OpAssign);
			CLLR_SPIRV_IMPL(OpCompare);

			CLLR_SPIRV_IMPL(OpValueCast);
			CLLR_SPIRV_IMPL(OpValueConstruct);
			CLLR_SPIRV_IMPL(OpConstructArg);
			CLLR_SPIRV_IMPL(OpValueDeref);
			CLLR_SPIRV_IMPL(OpValueExpr);
			CLLR_SPIRV_IMPL(OpValueExprUnary);
			CLLR_SPIRV_IMPL(OpValueInvokePos);
			CLLR_SPIRV_IMPL(OpValueInvokeSize);
			CLLR_SPIRV_IMPL(OpValueLitArray);
			CLLR_SPIRV_IMPL(OpLitArrayElem);
			CLLR_SPIRV_IMPL(OpValueLitBool);
			CLLR_SPIRV_IMPL(OpValueLitFloat);
			CLLR_SPIRV_IMPL(OpValueLitInt);
			CLLR_SPIRV_IMPL(OpValueLitStr);
			CLLR_SPIRV_IMPL(OpValueMember);
			CLLR_SPIRV_IMPL(OpValueNull);
			CLLR_SPIRV_IMPL(OpValueReadVar);
			CLLR_SPIRV_IMPL(OpValueSign);
			CLLR_SPIRV_IMPL(OpValueSubarray);
			CLLR_SPIRV_IMPL(OpValueUnsign);
			CLLR_SPIRV_IMPL(OpValueZero);

			CLLR_SPIRV_IMPL(OpReturn);
			CLLR_SPIRV_IMPL(OpReturnValue);
			CLLR_SPIRV_IMPL(OpDiscard);

		}
		
		class SPIRVOutAssembler : cllr::OutAssembler<uint32_t>
		{
		private:
			const uptr<spirv::CodeSection> spvHeader = SPIRV_CODE_SECTION(spirv::SSAKind::HEADER, this, SPIRVOpList{
				spirv::OpCapability(),
				spirv::OpExtension()
			});
			const uptr<spirv::CodeSection> spvImports = SPIRV_CODE_SECTION(spirv::SSAKind::IMPORT, this, SPIRVOpList{
				spirv::OpExtInstImport()
			});
			const uptr<spirv::CodeSection> spvMisc = SPIRV_CODE_SECTION(spirv::SSAKind::HEADER2, this, SPIRVOpList{
				spirv::OpMemoryModel(),
				spirv::OpEntryPoint(),
				spirv::OpExecutionMode(),
				spirv::OpExecutionModeId()
			});
			const uptr<spirv::CodeSection> spvDebug = SPIRV_CODE_SECTION(spirv::SSAKind::DEBUG, this, SPIRVOpList{
				spirv::OpString(),
				spirv::OpSource(),
				spirv::OpSourceExtension(),
				spirv::OpSourceContinued(),
				spirv::OpName(),
				spirv::OpMemberName(),
				spirv::OpModuleProcessed()
			});
			const uptr<spirv::CodeSection> spvTypes = SPIRV_CODE_SECTION(spirv::SSAKind::TYPE, this, SPIRVOpList{
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
			const uptr<spirv::CodeSection> spvConsts = SPIRV_CODE_SECTION(spirv::SSAKind::CONST, this, SPIRVOpList{
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
			uint32_t nextSSA = 1;
			HashMap<cllr::SSA, spirv::SSA> ssaAliases;

			std::vector<spirv::SSAEntry> ssaEntries;
			std::vector<spirv::SSAKind> ssaToSection;
			std::set<spirv::Capability> capabilities;
			std::vector<std::string> extensions;

			HashMap<std::string, spirv::SSA> instructions;

			spirv::AddressingModel addrModel = spirv::AddressingModel::Logical;
			spirv::MemoryModel memModel = spirv::MemoryModel::GLSL450;

			OutImpls<SPIRVOutFn> impls = {};

			constexpr uint32_t maxSSA() const
			{
				return nextSSA + 1;
			}

		public:
			std::vector<spirv::EntryPoint> shaderEntries;

			const uptr<spirv::CodeSection> decs = SPIRV_CODE_SECTION(spirv::SSAKind::DECORATION, this, SPIRVOpList{
				spirv::OpDecorate(),
				spirv::OpGroupDecorate(),
				spirv::OpGroupMemberDecorate(),
				spirv::OpMemberDecorate(),
				spirv::OpMemberDecorateString(),
				spirv::OpDecorationGroup()
			});
			const uptr<spirv::CodeSection> gloVars = SPIRV_CODE_SECTION(spirv::SSAKind::GLOBAL_VAR, this, SPIRVOpList{
				spirv::OpVariable()
			});
			const uptr<spirv::CodeSection> main = SPIRV_CODE_SECTION(spirv::SSAKind::MAIN, this, SPIRVOpList{});
			
			spirv::TypeSection types = spirv::TypeSection(this);
			spirv::ConstSection consts = spirv::ConstSection(this);

			spirv::SpvIO builtins = spirv::SpvIO(this);

			SPIRVOutAssembler();
			virtual ~SPIRVOutAssembler() {}

			uptr<std::vector<spirv::SSA>> translateCLLR(ref<cllr::Assembler> cllrAsm, ref<std::vector<sptr<cllr::Instruction>>> code) override;

			void searchAheadCLLR(ref<std::vector<spirv::SSA>> outIDs, std::vector<cllr::Opcode> ops, size_t off, uint32_t count, ref<cllr::Assembler> cllrAsm, std::function<cllr::SSA(sptr<cllr::Instruction>)> filter);

			spirv::SSA createSSA();

			spirv::SSA toSpvID(cllr::SSA ssa);

			spirv::SSAKind getSection(spirv::SSA id);

			void setSection(spirv::SSA id, spirv::SSAKind sec);

			void setSpvSSA(cllr::SSA in, spirv::SSA out);

			void setOpForSSA(spirv::SSA id, spirv::SpvOp op);

			spirv::SpvOp opFor(spirv::SSA id);

			void addExt(std::string ext);

			SSA addImport(std::string instructions);

			void setMemoryModel(spirv::AddressingModel addr, spirv::MemoryModel mem);

		};

	}

}
