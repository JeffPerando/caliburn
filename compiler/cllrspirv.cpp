
#include "cllrspirv.h"

using namespace caliburn;

uptr<std::vector<uint32_t>> cllr::SPIRVOutAssembler::translateCLLR(ref<cllr::Assembler> cllrAsm, ref<std::vector<sptr<cllr::Instruction>>> code)
{
	for (size_t off = 0; off < code.size(); ++off)
	{
		auto const& i = code[off];

		auto fn = (impls[(uint32_t)i->op]);
		(*fn)(target, *i, off, cllrAsm, *this);

	}

	auto shader = new_uptr<std::vector<uint32_t>>();
	
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
	for (auto const& entry : entries)
	{
		spvMisc->push(spirv::OpEntryPoint(), 0, { (uint32_t)entry.type, entry.func });
		spvMisc->pushRaw(entry.io);
	}

	//TODO insert execution modes

	//TODO insert debug instructions

	auto codeSecs = { &spvHeader, &spvImports, &spvMisc, &decs, &types, &consts, &spvGloVars, &main };

	size_t len = 0;
	for (auto const& sec : codeSecs)
	{
		len += (*sec)->code.size();
	}

	shader->reserve(shader->size() + len);

	for (auto const& sec : codeSecs)
	{
		(*sec)->dump(*shader);
	}

	return shader;
}

spirv::SSA cllr::SPIRVOutAssembler::createSSA()
{
	auto entry = spirv::SSAEntry{ ssa, spirv::OpNop() };

	++ssa;

	ssaEntries.push_back(entry);

	return entry.ssa;
}

spirv::SSA cllr::SPIRVOutAssembler::toSpvID(cllr::SSA ssa)
{
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

spirv::SSA cllr::SPIRVOutAssembler::addGlobalVar(SSA type, spirv::StorageClass stClass, SSA init)
{
	auto id = createSSA();
	spvGloVars->push(spirv::OpVariable(), id, { (uint32_t)stClass, init });
	return id;
}

void cllr::SPIRVOutAssembler::setMemoryModel(spirv::AddressingModel addr, spirv::MemoryModel mem)
{
	addrModel = addr;
	memModel = mem;
}

void cllr::SPIRVOutAssembler::addEntryPoint(SSA fn, spirv::ExecutionModel type, ref<std::vector<uint32_t>> ios)
{
	entries.push_back(spirv::EntryPoint{fn, type, std::vector(ios)});
}

//==========================================
//CLLR -> SPIR-V functions beyond this point
//==========================================

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpUnknown)
{
	return;//just don't
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpKernel)
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
		{ShaderType::MESH, spirv::ExecutionModel::MeshEXT},
	};
	
	auto type = (ShaderType)i.refs[0];
	auto ex = exModels.find(type)->second;

	InstructionVec cllrIns;
	std::vector<cllr::Opcode> ops = { Opcode::VAR_SHADER_IN, Opcode::VAR_SHADER_OUT };

	in.findAll(cllrIns, ops, off + 1, i.refs[1]);

	std::vector<uint32_t> ios;
	for (auto const& i : cllrIns)
	{
		ios.push_back(out.toSpvID(i->index));
	}

	out.addEntryPoint(out.toSpvID(i.index), ex, ios);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpKernelEnd)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpFunction)
{
	auto id = out.toSpvID(i.index);
	out.main->push(spirv::OpFunction(), id, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpFunctionEnd)
{
	out.main->push(spirv::OpFunctionEnd(), 0, {});
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarLocal)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarGlobal)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarFuncArg)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarShaderIn)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarShaderOut)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarDescriptor)
{

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCall)
{
	
}

//CLLR_SPIRV_IMPL(cllr::spirv_impl::OpDispatch){}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCallArg)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeVoid)
{
	auto id = out.toSpvID(i.index);

	out.types->push(spirv::OpTypeVoid(), id, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeFloat)
{
	auto id = out.toSpvID(i.index);

	out.types->push(spirv::OpTypeFloat(), id, { i.operands[0] });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeInt)
{
	auto id = out.toSpvID(i.index);

	out.types->push(spirv::OpTypeInt(), id, { i.operands[0], i.operands[1] });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeArray)//TODO decide on OpTypeRuntimeArray vs. OpTypeArray
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeVector)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	out.types->push(spirv::OpTypeVector(), id, { t, i.operands[0] });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeMatrix)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeStruct)
{
	auto id = out.toSpvID(i.index);

	std::vector<uint32_t> members;
	cllr::InstructionVec is;

	//It's tempting to replace this with actually implementing OpStructMember
	//BUT, I might change the spec to make adding new members less fidgety.
	//Also, new SPIR-V type system coming soon.
	in.findAll(is, { Opcode::STRUCT_MEMBER }, off + 1, i.operands[0]);

	for (auto& i : is)
	{
		members.push_back(out.toSpvID(i->index));
	}

	out.types->push(spirv::OpTypeStruct(i.operands[0]), id, members);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpStructMember) {} //we just search ahead

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpStructEnd) {} //this is just here for validation I guess

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeBool)
{
	auto id = out.toSpvID(i.index);

	out.types->push(spirv::OpTypeBool(), id, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypePtr)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	out.types->push(spirv::OpTypePointer(), id, { (uint32_t)spirv::StorageClass::Generic, t});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeTuple)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeString)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLabel)
{
	out.main->push(spirv::OpLabel(), out.toSpvID(i.index), {});

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

	out.main->push(spirv::OpBranchConditional(0), 0, { cond, t, f });

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

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCompare)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueArrayLit)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueBoolLit)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueCast)//TODO get value pointers if needed
{
	auto lhs = out.toSpvID(i.refs[0]);

	out.main->push(spirv::OpBitcast(), out.toSpvID(i.index), { lhs });//TODO get output type

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueDeref)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueDescriptor)//Not needed
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExpr)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExprUnary)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueFloatLit)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInit)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInstanceof)//WHAT.
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueIntLit)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInvokePos)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInvokeSize)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueMember)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueNull)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.refs[0]);

	out.main->push(spirv::OpConstantNull(), id, { t });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueStrLit)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueSubarray)
{
	
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueVariable)
{
	
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
	out.main->push(spirv::OpKill(), 0, {});

}
