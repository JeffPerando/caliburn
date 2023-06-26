
#include "spirv/cllrspirv.h"

#include "cinq.h"
#include "langcore.h"
#include "syntax.h"

using namespace caliburn;

uptr<std::vector<uint32_t>> cllr::SPIRVOutAssembler::translateCLLR(ref<cllr::Assembler> cllrAsm, ref<std::vector<sptr<cllr::Instruction>>> code)
{
	for (size_t off = 0; off < code.size(); ++off)
	{
		auto const& i = code[off];

		auto fn = (impls[(uint32_t)i->op]);
		(*fn)(target, *i, off, cllrAsm, *this);

	}

	types.dump(*spvTypes);
	consts.dump(*spvConsts);

	//Magic numbers
	spvHeader->pushRaw({ spirv::SPIRV_FILE_MAGIC_NUMBER , spirv::Version(1, 5), spirv::CALIBURN_GENERATOR_MAGIC_NUMBER, maxSSA(), 0});

	//Capabilities
	for (auto cap : capabilities)
	{
		spvHeader->push(spirv::OpCapability(), 0, { (uint32_t)cap });
	}

	//Extensions
	for (auto const& ext : extensions)
	{
		spvHeader->pushRaw({ spirv::OpExtension(spirv::SpvStrLen(ext)) });
		spvHeader->pushStr(ext);
	}

	//Imports (see spvImports)

	//from hereon we use spvMisc

	//Memory model
	spvMisc->push(spirv::OpMemoryModel(), 0, { (uint32_t)addrModel, (uint32_t)memModel });

	//Entry points
	for (auto const& entry : shaderEntries)
	{
		spvMisc->push(spirv::OpEntryPoint(), 0, { (uint32_t)entry.type, entry.func });
		spvMisc->pushRaw(entry.io);

	}

	//TODO insert execution modes

	//TODO insert debug instructions

	auto codeSecs = { &spvHeader, &spvImports, &spvMisc, &spvDebug, &decs, &spvTypes, &spvConsts, &gloVars, &main };

	size_t len = 0;
	for (auto const& sec : codeSecs)
	{
		len += (*sec)->code.size();
	}

	auto shader = new_uptr<std::vector<uint32_t>>();

	shader->reserve(shader->size() + len);

	for (auto const& sec : codeSecs)
	{
		(*sec)->dump(*shader);
	}

	return shader;
}

spirv::SSA cllr::SPIRVOutAssembler::createSSA()
{
	auto entry = spirv::SSAEntry{ nextSSA, spirv::OpNop() };

	++nextSSA;

	ssaEntries.push_back(entry);

	return entry.ssa;
}

spirv::SSA cllr::SPIRVOutAssembler::toSpvID(cllr::SSA ssa)
{
	if (ssa == 0)
	{
		return 0;
	}

	auto spvSSA = ssaAliases.find(ssa);

	if (spvSSA != ssaAliases.end())
	{
		return spvSSA->second;
	}

	auto nextSpvSSA = createSSA();
	
	ssaAliases.emplace(ssa, nextSpvSSA);

	return nextSpvSSA;
}

void cllr::SPIRVOutAssembler::setOpForSSA(spirv::SSA id, spirv::SpvOp op)
{
	if (id == 0 || op == spirv::OpNop())
	{
		return;
	}

	auto& entry = ssaEntries.at(id);

	if (entry.instruction != spirv::OpNop())
	{
		//TODO complain
		return;
	}

	entry.instruction = op;

}

void cllr::SPIRVOutAssembler::setSpvSSA(cllr::SSA in, spirv::SSA out)
{
	auto found = ssaAliases.find(in);

	if (found != ssaAliases.end())
	{
		//TODO complain
		return;
	}

	ssaAliases.emplace(in, out);

}

spirv::SpvOp cllr::SPIRVOutAssembler::opFor(spirv::SSA id)
{
	if (id >= ssaEntries.size())
	{
		return spirv::OpNop();
	}

	return ssaEntries.at(id).instruction;
}

void cllr::SPIRVOutAssembler::addExt(std::string ext)
{
	extensions.push_back(ext);
}

spirv::SSA cllr::SPIRVOutAssembler::addImport(std::string instructions)
{
	auto id = createSSA();

	spvImports->push(spirv::OpExtInstImport(spirv::SpvStrLen(instructions)), id, {});
	spvImports->pushStr(instructions);

	return id;
}

void cllr::SPIRVOutAssembler::setMemoryModel(spirv::AddressingModel addr, spirv::MemoryModel mem)
{
	addrModel = addr;
	memModel = mem;
}

//==========================================
//CLLR -> SPIR-V functions beyond this point
//==========================================

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpUnknown)
{
	return;//just don't
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpShaderStage)
{
	auto exModels = std::map<ShaderType, spirv::ExecutionModel>{
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
	
	auto type = (ShaderType)i.operands[0];
	auto ex = exModels.find(type)->second;

	InstructionVec cllrIns;
	
	in.findAll(cllrIns, { Opcode::VAR_SHADER_IN, Opcode::VAR_SHADER_OUT }, off + 1, i.operands[1]);

	auto ios = cinq::map<sptr<cllr::Instruction>, spirv::SSA>(cllrIns, lambda(auto i) {
		return out.toSpvID(i->index);
	});

	out.shaderEntries.push_back(spirv::EntryPoint
	{
		out.toSpvID(i.index),
		ex,
		ios
	});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpShaderStageEnd)
{
	/*
	All this code does is make a main() which forwards the output from the Caliburn shader to its proper builtin output
	For instance:
	void main()
	{
		out.gl_Position = cbrn_frag();
	}
	*/

	auto& code = *out.main.get();
	auto& types = out.types;

	auto& lastShaderEntry = out.shaderEntries.at(out.shaderEntries.size() - 1);
	auto const shaderModel = lastShaderEntry.type;

	if (shaderModel == spirv::ExecutionModel::GLCompute || shaderModel == spirv::ExecutionModel::Kernel)
	{
		return;//we don't care about compute shaders
		//also this code shouldn't even execute for a compute shader, but w/e
	}

	auto v = types.findOrMake(spirv::OpTypeVoid(), {});
	auto fn_v_0 = types.findOrMake(spirv::OpTypeFunction(0), { v });

	auto i32 = types.findOrMake(spirv::OpTypeInt(), { 32, 1 });
	auto fp = types.findOrMake(spirv::OpTypeFloat(), { 32 });

	spirv::SSA outType = 0;
	out.builtins.getOutputFor(shaderModel, outType);

	//void main() {
	auto mainID = out.createSSA();
	code.pushTyped(spirv::OpFunction(), v, mainID, { fn_v_0, (uint32_t)spirv::FuncControl() });

	auto callID = out.createSSA();
	code.pushTyped(spirv::OpFunctionCall(), outType, callID, { lastShaderEntry.func });

	auto accessID = out.createSSA();
	//TODO doesn't work for fragment shaders (oops)
	code.pushTyped(spirv::OpAccessChain(), outType, accessID, { outType, out.consts.findOrMake(i32, 0U)});

	code.push(spirv::OpStore(), 0, { accessID, callID });

	//}
	code.push(spirv::OpFunctionEnd(), 0, {});

	//Change the entry point so that the shader works correctly
	lastShaderEntry.func = mainID;

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpFunction)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	InstructionVec args;
	in.findAll(args, { Opcode::VAR_FUNC_ARG }, off + 1, i.operands[0]);

	auto fnArgs = cinq::map<sptr<Instruction>, spirv::SSA>(args, lambda(auto i) {
		return out.toSpvID(i->refs[0]);
	});

	//put the return type at the start so we can just pass the whole vector
	fnArgs.emplace(fnArgs.begin(), t);

	auto fnSig = out.types.findOrMake(spirv::OpTypeFunction((uint32_t)fnArgs.size() - 1), fnArgs);

	out.main->pushTyped(spirv::OpFunction(), t, id, { fnSig });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarFuncArg)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	out.main->pushTyped(spirv::OpFunctionParameter(), t, id, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpFunctionEnd)
{
	out.main->push(spirv::OpFunctionEnd(), 0, {});
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarLocal)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	auto sc = spirv::StorageClass::Function;
	
	auto mods = StmtModifiers{ i.operands[0] };
	
	if (mods.SHARED)
	{
		sc = spirv::StorageClass::Workgroup;
	}

	out.main->pushVar(t, id, sc, out.toSpvID(i.refs[1]));
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarGlobal)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	if (i.refs[1] == 0)
	{
		//CRITICAL ERROR
		return;
	}

	out.gloVars->pushVar(t, id, spirv::StorageClass::CrossWorkgroup, out.toSpvID(i.refs[1]));

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarShaderIn)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	out.gloVars->pushVar(t, id, spirv::StorageClass::Input, 0);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarShaderOut)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	out.gloVars->pushVar(t, id, spirv::StorageClass::Output, 0);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarDescriptor)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	out.gloVars->pushVar(t, id, spirv::StorageClass::Uniform, 0);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCall)
{
	auto id = out.toSpvID(i.index);
	auto fnID = out.toSpvID(i.refs[0]);
	auto retType = out.toSpvID(i.outType);

	InstructionVec args;
	in.findAll(args, { Opcode::CALL_ARG }, off + 1, i.operands[0]);

	auto fnArgs = cinq::map<sptr<Instruction>, spirv::SSA>(args, lambda(auto i) { return out.toSpvID(i->refs[0]); });

	//push the function ID to pass the whole vector
	fnArgs.emplace(fnArgs.begin(), fnID);

	out.main->pushTyped(spirv::OpFunctionCall((uint32_t)fnArgs.size() - 1), id, retType, fnArgs);

}

//CLLR_SPIRV_IMPL(cllr::spirv_impl::OpDispatch){}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCallArg) {} //search-ahead

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeVoid)
{
	auto id = out.toSpvID(i.index);

	out.types.pushNew(spirv::OpTypeVoid(), id, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeFloat)
{
	auto id = out.toSpvID(i.index);

	out.types.pushNew(spirv::OpTypeFloat(), id, { i.operands[0] });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeInt)
{
	auto id = out.toSpvID(i.index);

	out.types.pushNew(spirv::OpTypeInt(), id, { i.operands[0], i.operands[1] });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeArray)//TODO decide on OpTypeRuntimeArray vs. OpTypeArray
{
	auto id = out.toSpvID(i.index);
	auto inner = out.toSpvID(i.refs[0]);

	out.types.pushNew(spirv::OpTypeArray(), id, { inner, i.operands[0]});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeVector)
{
	auto id = out.toSpvID(i.index);
	auto inner = out.toSpvID(i.refs[0]);

	out.types.pushNew(spirv::OpTypeVector(), id, { inner, i.operands[0] });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeMatrix)
{
	auto elemType = out.toSpvID(i.refs[0]);
	auto colType = out.types.findOrMake(spirv::OpTypeVector(), { elemType, i.operands[0] });

	auto id = out.toSpvID(i.index);

	out.types.pushNew(spirv::OpTypeMatrix(), id, { colType, i.operands[1]});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeStruct)
{
	auto id = out.toSpvID(i.index);

	cllr::InstructionVec is;

	//It's tempting to replace this with actually implementing OpStructMember
	//BUT, I might change the spec to make adding new members less fidgety.
	in.findAll(is, { Opcode::STRUCT_MEMBER }, off + 1, i.operands[0]);

	auto members = cinq::map<sptr<Instruction>, spirv::SSA>(is, lambda(auto mem) { return out.toSpvID(mem->refs[0]); });
	
	out.types.pushNew(spirv::OpTypeStruct(i.operands[0]), id, members);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpStructMember) {} //we just search ahead

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpStructEnd) {} //this is just here for validation I guess

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeBool)
{
	auto id = out.toSpvID(i.index);

	out.types.pushNew(spirv::OpTypeBool(), id);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypePtr)
{
	auto id = out.toSpvID(i.index);
	auto inner = out.toSpvID(i.refs[0]);

	out.types.pushNew(spirv::OpTypePointer(), id, { (uint32_t)spirv::StorageClass::Generic, inner });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeTuple)
{
	auto a = out.toSpvID(i.refs[0]);
	auto b = out.toSpvID(i.refs[1]);

	auto id = out.types.findOrMake(spirv::OpTypeStruct(), { a, b });

	out.setSpvSSA(i.index, id);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeString)
{
	//TODO figure out strings in SPIR-V
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLabel)
{
	out.main->push(spirv::OpLabel(), out.toSpvID(i.index));

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpJump)
{
	out.main->push(spirv::OpBranch(), 0, { out.toSpvID(i.refs[0]) });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpJumpCond)
{
	auto cond = out.toSpvID(i.refs[0]);
	auto t = out.toSpvID(i.refs[1]);
	auto f = out.toSpvID(i.refs[2]);

	out.main->push(spirv::OpBranchConditional(1), 0, { cond, t, f });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLoop)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpAssign)
{
	auto lhs = out.toSpvID(i.refs[0]);
	auto rhs = out.toSpvID(i.refs[1]);

	out.main->push(spirv::OpStore(0), 0, { lhs, rhs });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCompare) //TODO finish
{
	auto id = out.toSpvID(i.index);
	auto cllrOp = (Operator)i.operands[0];

	auto cllrLHS = i.refs[0];
	auto cllrRHS = i.refs[1];

	auto lhs = out.toSpvID(cllrLHS);
	auto rhs = out.toSpvID(cllrRHS);

	auto op = spirv::OpNop();

	switch (cllrOp)
	{
	case Operator::COMP_EQ: op = spirv::OpIEqual(); break;
	case Operator::COMP_NEQ: op = spirv::OpINotEqual(); break;
	case Operator::COMP_GT: op = spirv::OpSGreaterThan(); break;
	case Operator::COMP_GTE: op = spirv::OpSGreaterThanEqual(); break;
	case Operator::COMP_LT: op = spirv::OpSLessThan(); break;
	case Operator::COMP_LTE: op = spirv::OpSLessThanEqual(); break;
	}

	//out.main->push(op, id, { lhs, rhs });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueCast)//TODO check impl
{
	auto lhs = out.toSpvID(i.refs[0]);

	out.main->push(spirv::OpBitcast(), out.toSpvID(i.index), { lhs });//TODO get output type

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueDeref)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExpr)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExprUnary)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInvokePos)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInvokeSize)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitArray)
{

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitBool)
{
	//Don't wory about find + making types in literals; it'll probably be made elsewhere

	auto outID = out.consts.findOrMake(out.toSpvID(i.outType), i.operands[0]);

	out.setSpvSSA(i.index, outID);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitFloat)
{
	auto str = in.getString(i.operands[0]);
	
	//TODO consider a better algorithm
	auto v = std::stof(str);

	auto outID = out.consts.findOrMakeFP32(out.toSpvID(i.outType), v);

	out.setSpvSSA(i.index, outID);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitInt)
{
	auto outID = out.consts.findOrMake(out.toSpvID(i.outType), i.operands[0], i.operands[1]);

	out.setSpvSSA(i.index, outID);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitStr)
{
	//TODO figure out strings within SPIR-V
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueMember)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueNull)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.outType);

	out.main->push(spirv::OpConstantNull(), id, { t });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueReadVar)
{
	auto outVal = out.toSpvID(i.index);
	auto inVal = out.toSpvID(i.refs[0]);
	auto outType = out.toSpvID(i.outType);

	out.main->pushTyped(spirv::OpLoad(0), outType, outVal, { inVal });
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueSubarray)
{
	
}

/*
Now, don't shoot me. DO NOT SHOOT ME.

but uh
OpConstantNull fulfills the Caliburn spec. Seemingly.
TODO: test against the spec. Worried about arrays, vectors.
*/
CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueZero)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.outType);

	out.main->push(spirv::OpConstantNull(), id, { t });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpReturn)
{
	auto id = out.toSpvID(i.index);

	out.main->push(spirv::OpReturn(), id, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpReturnValue)
{
	auto id = out.toSpvID(i.index);
	auto val = out.toSpvID(i.refs[0]);

	out.main->push(spirv::OpReturnValue(), id, { val });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpDiscard)
{
	//out.main->push(spirv::OpTerminateInvocation(), 0, {});
	//I know it's deprecated, but like... eh.
	out.main->push(spirv::OpKill(), 0, {});

}
