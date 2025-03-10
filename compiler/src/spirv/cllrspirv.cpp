
#include "spirv/cllrspirv.h"

#include "cllr/cllrfind.h"

#include "cinq.h"
#include "langcore.h"
#include "syntax.h"

using namespace caliburn;

cllr::SPIRVOutAssembler::SPIRVOutAssembler(sptr<const CompilerSettings> cs) : OutAssembler(cs, HostTarget::GPU)
{
	ssaEntries.reserve(128);
	ssaToSection.reserve(128);

	//here we go...

	setImpl(Opcode::SHADER_STAGE, spirv_impl::OpShaderStage);
	setImpl(Opcode::SHADER_STAGE_END, spirv_impl::OpShaderStageEnd);

	setImpl(Opcode::FUNCTION, spirv_impl::OpFunction);
	setImpl(Opcode::VAR_FUNC_ARG, spirv_impl::OpVarFuncArg);
	setImpl(Opcode::FUNCTION_END, spirv_impl::OpFunctionEnd);

	setImpl(Opcode::VAR_LOCAL, spirv_impl::OpVarLocal);
	setImpl(Opcode::VAR_GLOBAL, spirv_impl::OpVarGlobal);
	setImpl(Opcode::VAR_SHADER_IN, spirv_impl::OpVarShaderIn);
	setImpl(Opcode::VAR_SHADER_OUT, spirv_impl::OpVarShaderOut);
	setImpl(Opcode::VAR_DESCRIPTOR, spirv_impl::OpVarDescriptor);

	setImpl(Opcode::CALL, spirv_impl::OpCall);
	setImpl(Opcode::CALL_ARG, spirv_impl::OpCallArg);

	setImpl(Opcode::TYPE_VOID, spirv_impl::OpTypeVoid);
	setImpl(Opcode::TYPE_FLOAT, spirv_impl::OpTypeFloat);
	setImpl(Opcode::TYPE_INT_SIGN, spirv_impl::OpTypeIntSign);
	setImpl(Opcode::TYPE_INT_UNSIGN, spirv_impl::OpTypeIntUnsign);
	setImpl(Opcode::TYPE_ARRAY, spirv_impl::OpTypeArray);
	setImpl(Opcode::TYPE_VECTOR, spirv_impl::OpTypeVector);
	setImpl(Opcode::TYPE_MATRIX, spirv_impl::OpTypeMatrix);
	setImpl(Opcode::TYPE_TEXTURE, spirv_impl::OpTypeTexture);

	setImpl(Opcode::TYPE_STRUCT, spirv_impl::OpTypeStruct);
	setImpl(Opcode::STRUCT_MEMBER, spirv_impl::OpStructMember);
	setImpl(Opcode::STRUCT_END, spirv_impl::OpStructEnd);

	setImpl(Opcode::TYPE_BOOL, spirv_impl::OpTypeBool);
	setImpl(Opcode::TYPE_PTR, spirv_impl::OpTypePtr);
	setImpl(Opcode::TYPE_TUPLE, spirv_impl::OpTypeTuple);
	//setImpl(Opcode::TYPE_STRING, spirv_impl::OpTypeString);

	setImpl(Opcode::LABEL, spirv_impl::OpLabel);
	setImpl(Opcode::JUMP, spirv_impl::OpJump);
	setImpl(Opcode::JUMP_COND, spirv_impl::OpJumpCond);
	setImpl(Opcode::LOOP, spirv_impl::OpLoop);

	setImpl(Opcode::ASSIGN, spirv_impl::OpAssign);
	setImpl(Opcode::COMPARE, spirv_impl::OpCompare);
	setImpl(Opcode::VALUE_CAST, spirv_impl::OpValueCast);
	setImpl(Opcode::VALUE_CONSTRUCT, spirv_impl::OpValueConstruct);
	setImpl(Opcode::CONSTRUCT_ARG, spirv_impl::OpConstructArg);
	setImpl(Opcode::VALUE_DEREF, spirv_impl::OpValueDeref);
	setImpl(Opcode::VALUE_EXPAND, spirv_impl::OpValueExpand);
	setImpl(Opcode::VALUE_EXPR, spirv_impl::OpValueExpr);
	setImpl(Opcode::VALUE_EXPR_UNARY, spirv_impl::OpValueExprUnary);
	setImpl(Opcode::VALUE_INVOKE_POS, spirv_impl::OpValueInvokePos);
	setImpl(Opcode::VALUE_INVOKE_SIZE, spirv_impl::OpValueInvokeSize);
	setImpl(Opcode::VALUE_LIT_ARRAY, spirv_impl::OpValueLitArray);
	setImpl(Opcode::LIT_ARRAY_ELEM, spirv_impl::OpLitArrayElem);
	setImpl(Opcode::VALUE_LIT_BOOL, spirv_impl::OpValueLitBool);
	setImpl(Opcode::VALUE_LIT_FP, spirv_impl::OpValueLitFloat);
	setImpl(Opcode::VALUE_LIT_INT, spirv_impl::OpValueLitInt);
	setImpl(Opcode::VALUE_LIT_STR, spirv_impl::OpValueLitStr);
	setImpl(Opcode::VALUE_MEMBER, spirv_impl::OpValueMember);
	setImpl(Opcode::VALUE_NULL, spirv_impl::OpValueNull);
	setImpl(Opcode::VALUE_READ_VAR, spirv_impl::OpValueReadVar);
	setImpl(Opcode::VALUE_SAMPLE, spirv_impl::OpValueSample);
	setImpl(Opcode::VALUE_SIGN, spirv_impl::OpValueSign);
	setImpl(Opcode::VALUE_SUBARRAY, spirv_impl::OpValueSubarray);
	setImpl(Opcode::VALUE_UNSIGN, spirv_impl::OpValueUnsign);
	setImpl(Opcode::VALUE_VEC_SWIZZLE, spirv_impl::OpValueVecSwizzle);
	setImpl(Opcode::VALUE_ZERO, spirv_impl::OpValueZero);

	setImpl(Opcode::RETURN, spirv_impl::OpReturn);
	setImpl(Opcode::RETURN_VALUE, spirv_impl::OpReturnValue);
	setImpl(Opcode::DISCARD, spirv_impl::OpDiscard);

	//Expression implementations
	exprImpls.emplace(Opcode::TYPE_FLOAT, spirv_impl::OpExprFloat);
	exprImpls.emplace(Opcode::TYPE_INT_SIGN, spirv_impl::OpExprIntSign);
	exprImpls.emplace(Opcode::TYPE_INT_UNSIGN, spirv_impl::OpExprIntUnsign);
	exprImpls.emplace(Opcode::TYPE_VECTOR, spirv_impl::OpExprVector);
	exprImpls.emplace(Opcode::TYPE_MATRIX, spirv_impl::OpExprMatrix);

}

std::vector<uint32_t> cllr::SPIRVOutAssembler::translateCLLR(in<cllr::Assembler> cllrAsm)
{
	auto const& code = cllrAsm.getCode();

	for (size_t off = 0; off < code.size(); ++off)
	{
		auto const& i = code.at(off);

		auto fn = (impls[(uint32_t)i.op]);
		(*fn)(i, off, cllrAsm, *this);

	}

	types.dump(*spvTypes);
	consts.dump(*spvConsts);

	//Capabilities
	for (auto cap : capabilities)
	{
		spvHeader->push(spirv::OpCapability(), 0, { SCAST<uint32_t>(cap) });
	}

	//Extensions
	for (in<std::string> ext : extensions)
	{
		spvHeader->pushRaw({ spirv::OpExtension(spirv::SpvStrLen(ext)) });
		spvHeader->pushStr(ext);
	}

	//Imports (see spvImports)

	//from hereon we use spvMisc

	//Memory model
	spvMisc->push(spirv::OpMemoryModel(), 0, { SCAST<uint32_t>(addrModel), SCAST<uint32_t>(memModel) });

	//Entry points
	for (auto& entry : shaderEntries)
	{
		if (entry.name.empty())
		{
			entry.name = (std::string("CBRN_").append(SHADER_TYPE_NAMES.at(cllrAsm.type)));
		}

		spvMisc->push(spirv::OpEntryPoint((uint32_t)entry.io.size() + spirv::SpvStrLen(entry.name)), 0, { SCAST<uint32_t>(entry.type), entry.func });
		spvMisc->pushStr(entry.name);
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

	std::vector<uint32_t> shader;

	shader.reserve(len);

	//Magic numbers
	shader.insert(shader.begin(), { spirv::SPIRV_FILE_MAGIC_NUMBER, spirv::Version(1, 5), spirv::CALIBURN_GENERATOR_MAGIC_NUMBER, maxSSA(), 0 });

	for (auto const& sec : codeSecs)
	{
		(*sec)->dump(shader);
	}

	return shader;
}

spirv::SSA cllr::SPIRVOutAssembler::createSSA()
{
	auto entry = spirv::SSAEntry{ nextSSA, spirv::OpNop() };

	++nextSSA;

	ssaEntries.push_back(entry);
	ssaToSection.push_back(spirv::SpvSection::UNKNOWN);

	return entry.ssa;
}

spirv::SSA cllr::SPIRVOutAssembler::toSpvID(cllr::SSA ssa)
{
	if (ssa == 0)
	{
		throw std::exception();
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

spirv::SpvSection cllr::SPIRVOutAssembler::getSection(spirv::SSA id)
{
	if (id >= ssaToSection.size())
		return spirv::SpvSection::UNKNOWN;

	return ssaToSection[id];
}

void cllr::SPIRVOutAssembler::setSection(spirv::SSA id, spirv::SpvSection sec)
{
	if (id == 0)
		return;//TODO complain

	if (id >= ssaToSection.size())
		return;//TODO complain

	if (ssaToSection[id] != spirv::SpvSection::UNKNOWN)
	{
		if (ssaToSection[id] != sec)
		{
			//TODO complain
		}

		return;
	}

	ssaToSection[id] = sec;

}

void cllr::SPIRVOutAssembler::setOpForSSA(spirv::SSA id, spirv::SpvOp op)
{
	if (id == 0 || op == spirv::OpNop())
	{
		return;
	}

	if (id >= ssaEntries.size())
	{
		throw std::exception("NO.");
	}

	auto& entry = ssaEntries.at(id);

	if (entry.instruction != spirv::OpNop())
	{
		//TODO complain
		return;
	}

	entry.ssa = id;
	entry.instruction = op;

}

void cllr::SPIRVOutAssembler::setSpvSSA(cllr::SSA inID, spirv::SSA outID)
{
	auto found = ssaAliases.find(inID);

	if (found != ssaAliases.end())
	{
		//TODO complain
		return;
	}

	ssaAliases.emplace(inID, outID);

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

spirv::SSA cllr::SPIRVOutAssembler::addImport(std::string ins)
{
	auto found = instructions.find(ins);

	if (found != instructions.end())
	{
		return found->second;
	}

	auto id = createSSA();

	spvImports->push(spirv::OpExtInstImport(spirv::SpvStrLen(ins)), id, {});
	spvImports->pushStr(ins);

	instructions.emplace(ins, id);

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
	auto id = outCode.toSpvID(i.index);
	auto type = (ShaderType)i.operands[0];
	auto ex = SHADER_EXECUTION_MODELS.find(type)->second;

	//TODO don't trust output type
	auto outType = outCode.toSpvID(i.refs[0]);

	auto fn_type = outCode.types.findOrMake(spirv::OpTypeFunction(0), { outType });

	spirv::FuncControl fnctrl;

	fnctrl.Inline = 1;

	outCode.main->pushTyped(spirv::OpFunction(), outType, id, { fnctrl, fn_type });

	outCode.shaderEntries.push_back(spirv::EntryPoint
		{
			id, ex, inCode.getString(i.operands[1])
		});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpShaderStageEnd)
{
	/*
	All this code does is make a main() which forwards the output from the Caliburn shader to its proper builtin output
	For instance:
	void main()
	{
		outCode.gl_Position = cbrn_frag();
	}
	*/

	auto& code = *outCode.main.get();
	auto& types = outCode.types;

	code.push(spirv::OpFunctionEnd(), 0, {});

	auto& entry = outCode.getCurrentEntry();

	if (entry.type == spirv::ExecutionModel::GLCompute || entry.type == spirv::ExecutionModel::Kernel)
	{
		return;//we don't care about compute shaders
		//also this code shouldn't even execute for a compute shader, but w/e
	}

	auto v = types.findOrMake(spirv::OpTypeVoid(), {});
	auto fn_v_0 = types.findOrMake(spirv::OpTypeFunction(0), { v });

	auto u32 = types.findOrMake(spirv::OpTypeInt(), { 32, 0 });
	//auto fp = types.findOrMake(spirv::OpTypeFloat(), { 32 });

	spirv::SSA outType = 0;
	auto outID = outCode.builtins.getOutputFor(entry.type, outType);

	//void main() {
	auto mainID = outCode.createSSA();
	code.pushTyped(spirv::OpFunction(), v, mainID, { spirv::FuncControl(), fn_v_0 });

	//shader call
	auto callID = outCode.createSSA();
	code.pushTyped(spirv::OpFunctionCall(0), outType, callID, { entry.func });

	auto accessID = outCode.createSSA();
	//TODO doesn't work for fragment shaders (oops)
	//gl_Position
	code.pushTyped(spirv::OpAccessChain(1), outType, accessID, { outID, outCode.consts.findOrMake(u32, 0U) });
	//=
	code.push(spirv::OpStore(0), 0, { accessID, callID });

	//}
	code.push(spirv::OpFunctionEnd(), 0, {});

	//Change the entry point so that the shader works correctly
	entry.func = mainID;

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpFunction)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.refs[0]);

	auto cllrFnArgs = cllr::Finder()
		.ops({ Opcode::VAR_FUNC_ARG })
		->setOffset(off + 1)
		->setLimit(i.operands[0])
		->find(inCode.getCode());

	auto fnArgs = cinq::map<Instruction, spirv::SSA>(cllrFnArgs, LAMBDA(in<Instruction> i) { return outCode.toSpvID(i.refs[0]); });

	//put the return type at the start so we can just pass the whole vector
	fnArgs.emplace(fnArgs.begin(), t);

	auto fnSig = outCode.types.findOrMake(spirv::OpTypeFunction((uint32_t)fnArgs.size() - 1), fnArgs);

	outCode.main->pushTyped(spirv::OpFunction(), t, id, { fnSig });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarFuncArg)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.refs[0]);

	outCode.main->pushTyped(spirv::OpFunctionParameter(), t, id, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpFunctionEnd)
{
	outCode.main->push(spirv::OpFunctionEnd(), 0, {});
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarLocal)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.refs[0]);

	auto sc = spirv::StorageClass::Function;

	auto mods = ExprModifiers{ i.operands[0] };

	if (mods.SHARED)
	{
		sc = spirv::StorageClass::Workgroup;
	}

	outCode.main->pushVar(t, id, sc, outCode.toSpvID(i.refs[1]));

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarGlobal)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.refs[0]);

	if (i.refs[1] == 0)
	{
		//CRITICAL ERROR
		return;
	}

	outCode.gloVars->pushVar(t, id, spirv::StorageClass::CrossWorkgroup, outCode.toSpvID(i.refs[1]));

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarShaderIn)
{
	auto id = outCode.toSpvID(i.index);
	auto innerType = outCode.toSpvID(i.refs[0]);

	auto constexpr sc = spirv::StorageClass::Input;

	outCode.gloVars->pushVar(outCode.types.typePtr(innerType, sc), id, sc, 0);
	outCode.decs->decorate(id, spirv::Decoration::Location, { i.operands[0] });

	outCode.getCurrentEntry().io.push_back(id);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarShaderOut)
{
	auto id = outCode.toSpvID(i.index);
	auto innerType = outCode.toSpvID(i.refs[0]);

	auto constexpr sc = spirv::StorageClass::Output;

	outCode.gloVars->pushVar(outCode.types.typePtr(innerType, sc), id, sc, 0);
	outCode.decs->decorate(id, spirv::Decoration::Location, { i.operands[0] });

	outCode.getCurrentEntry().io.push_back(id);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpVarDescriptor)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.refs[0]);

	outCode.gloVars->pushVar(t, id, spirv::StorageClass::Uniform, 0);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCall)
{
	auto id = outCode.toSpvID(i.index);
	auto fnID = outCode.toSpvID(i.refs[0]);
	auto retType = outCode.toSpvID(i.outType);

	auto cllrFnArgs = cllr::Finder()
		.ops({ Opcode::CALL_ARG })
		->references(i.index)
		->setOffset(off + 1)
		->setLimit(i.operands[0])
		->find(inCode.getCode());

	auto fnArgs = cinq::map<Instruction, spirv::SSA>(cllrFnArgs, LAMBDA(in<Instruction> i) { return outCode.toSpvID(i.refs[0]); });

	//push the function ID to pass the whole vector
	fnArgs.emplace(fnArgs.begin(), fnID);

	outCode.main->pushTyped(spirv::OpFunctionCall((uint32_t)fnArgs.size() - 1), id, retType, fnArgs);

}

//CLLR_SPIRV_IMPL(cllr::spirv_impl::OpDispatch){}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCallArg) {} //search-ahead

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeVoid)
{
	auto id = outCode.toSpvID(i.index);

	outCode.types.findOrMake(spirv::OpTypeVoid(), {}, id);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeFloat)
{
	auto fid = outCode.types.findOrMake(spirv::OpTypeFloat(), { i.operands[0] });

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeIntSign)
{
	auto fid = outCode.types.findOrMake(spirv::OpTypeInt(), { i.operands[0], 1 });

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeIntUnsign)
{
	auto fid = outCode.types.findOrMake(spirv::OpTypeInt(), { i.operands[0], 0 });

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeArray)//TODO decide on OpTypeRuntimeArray vs. OpTypeArray
{
	auto inner = outCode.toSpvID(i.refs[0]);

	auto fid = outCode.types.findOrMake(spirv::OpTypeRuntimeArray(), { inner });

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeVector)
{
	auto inner = outCode.toSpvID(i.refs[0]);

	auto fid = outCode.types.findOrMake(spirv::OpTypeVector(), { inner, i.operands[0] });

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeMatrix)
{
	auto elemType = outCode.toSpvID(i.refs[0]);
	auto colType = outCode.types.findOrMake(spirv::OpTypeVector(), { elemType, i.operands[0] });

	auto fid = outCode.types.findOrMake(spirv::OpTypeMatrix(), { colType, i.operands[1] });

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeTexture)
{
	spirv::SSA elemID = 0;
	uint32_t imgLen = 0;
	
	auto pixelID = outCode.toSpvID(i.refs[0]);

	spirv::Type pixelData, elemData;

	if (!outCode.types.findData(pixelID, pixelData))
	{
		//TODO complain
		return;
	}

	if (pixelData.opcode == spirv::OpTypeVector())
	{
		elemID = pixelData.operands[0];
		imgLen = pixelData.operands[1];

		if (!outCode.types.findData(elemID, elemData))
		{
			//TODO complain
			return;
		}

	}
	else
	{
		elemID = pixelID;
		imgLen = 1;

		elemData = pixelData;

	}

	spirv::Dim dim;

	static const HashMap<TextureKind, spirv::Dim> TEXTURE_DIM = {
		{TextureKind::_1D, spirv::Dim::_1D},
		{TextureKind::_2D, spirv::Dim::_2D},
		{TextureKind::_3D, spirv::Dim::_3D},
		{TextureKind::CUBEMAP, spirv::Dim::Cube}
	};

	if (auto d = TEXTURE_DIM.find(SCAST<TextureKind>(i.operands[0])); d != TEXTURE_DIM.end())
	{
		dim = d->second;
	}
	else
	{
		//TODO complain
		return;
	}

	auto fmt = spirv::ImageFormat::Unknown;

	if (elemData.opcode == spirv::OpTypeFloat())
	{
		auto const width = elemData.operands[0];

		if (width > 32)
		{
			//TODO complain
			return;
		}

		switch (width)
		{
		case 16: {
			switch (imgLen)
			{
			case 1: fmt = spirv::ImageFormat::R16f; break;
			case 2: fmt = spirv::ImageFormat::Rg16f; break;
			case 3: PASS;
			case 4: fmt = spirv::ImageFormat::Rgba16f; break;
			}
		}; break;
		case 32: {
			switch (imgLen)
			{
			case 1: fmt = spirv::ImageFormat::R32f; break;
			case 2: fmt = spirv::ImageFormat::Rg32f; break;
			case 3: PASS;
			case 4: fmt = spirv::ImageFormat::Rgba32f; break;
			}
		}; break;
		}

	}
	else if (elemData.opcode == spirv::OpTypeInt())
	{
		auto const width = elemData.operands[0];
		bool const sign = elemData.operands[1] != 0;

		if (width > 32)
		{
			//TODO complain
			return;
		}

		if (sign)
		{
			switch (width)
			{
			case 8: {
				switch (imgLen)
				{
				case 1: fmt = spirv::ImageFormat::R8i; break;
				case 2: fmt = spirv::ImageFormat::Rg8i; break;
				case 3: PASS;
				case 4: fmt = spirv::ImageFormat::Rgba8i; break;
				}
			}; break;
			case 16: {
				switch (imgLen)
				{
				case 1: fmt = spirv::ImageFormat::R16i; break;
				case 2: fmt = spirv::ImageFormat::Rg16i; break;
				case 3: PASS;
				case 4: fmt = spirv::ImageFormat::Rgba16i; break;
				}
			}; break;
			case 32: {
				switch (imgLen)
				{
				case 1: fmt = spirv::ImageFormat::R32i; break;
				case 2: fmt = spirv::ImageFormat::Rg32i; break;
				case 3: PASS;
				case 4: fmt = spirv::ImageFormat::Rgba32i; break;
				}
			}; break;
			}

		}
		else
		{
			switch (width)
			{
			case 8: {
				switch (imgLen)
				{
				case 1: fmt = spirv::ImageFormat::R8ui; break;
				case 2: fmt = spirv::ImageFormat::Rg8ui; break;
				case 3: PASS;
				case 4: fmt = spirv::ImageFormat::Rgba8ui; break;
				}
			}; break;
			case 16: {
				switch (imgLen)
				{
				case 1: fmt = spirv::ImageFormat::R16ui; break;
				case 2: fmt = spirv::ImageFormat::Rg16ui; break;
				case 3: PASS;
				case 4: fmt = spirv::ImageFormat::Rgba16ui; break;
				}
			}; break;
			case 32: {
				switch (imgLen)
				{
				case 1: fmt = spirv::ImageFormat::R32ui; break;
				case 2: fmt = spirv::ImageFormat::Rg32ui; break;
				case 3: PASS;
				case 4: fmt = spirv::ImageFormat::Rgba32ui; break;
				}
			}; break;
			}

		}

	}
	else
	{
		//TODO complain
		return;
	}

	auto tid = outCode.types.typeSampleImg(elemID, dim, fmt, 0, 0, 0);

	outCode.setSpvSSA(i.index, tid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeStruct)
{
	//It's tempting to replace this with actually implementing OpStructMember
	//BUT, I might change the spec to make adding new members less fidgety.
	auto cllrMembers = cllr::Finder()
		.ops({ Opcode::STRUCT_MEMBER })
		->setOffset(off + 1)
		->references(i.index)
		->setLimit(i.operands[0])
		->find(inCode.getCode());

	auto members = cinq::map<Instruction, spirv::SSA>(cllrMembers, LAMBDA(in<Instruction> i) { return outCode.toSpvID(i.refs[0]); });

	auto memCount = (uint32_t)members.size();

	auto fid = outCode.types.findOrMake(spirv::OpTypeStruct(memCount), members);

	if (i.operands[0] != memCount)
	{
		//TODO complain
	}

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpStructMember) {} //we just search ahead

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpStructEnd) {} //this is just here for validation I guess

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeBool)
{
	auto fid = outCode.types.findOrMake(spirv::OpTypeBool(), {});

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypePtr)
{
	auto inner = outCode.toSpvID(i.refs[0]);

	auto fid = outCode.types.findOrMake(spirv::OpTypePointer(), { (uint32_t)spirv::StorageClass::Generic, inner });

	outCode.setSpvSSA(i.index, fid);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeTuple)
{
	auto a = outCode.toSpvID(i.refs[0]);
	auto b = outCode.toSpvID(i.refs[1]);

	auto fid = outCode.types.findOrMake(spirv::OpTypeStruct(2), { a, b });

	outCode.setSpvSSA(i.index, fid);

}

/*
CLLR_SPIRV_IMPL(cllr::spirv_impl::OpTypeString)
{
	//TODO figure out strings inCode SPIR-V
}
*/

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpScopeBegin)
{
	outCode.main->push(spirv::OpLabel(), outCode.toSpvID(i.index));

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpScopeEnd)
{
	auto id = outCode.toSpvID(i.index);

	//TODO turn this into a function somewhere
	//these are all block-terminating instructions
	switch (outCode.main->getLastOp().op)
	{
	case spirv::OpReturn().op: break;
	case spirv::OpReturnValue().op: break;
	case spirv::OpKill().op: break;
	case spirv::OpUnreachable().op: break;
	case spirv::OpTerminateInvocation().op: break;
	case spirv::OpBranch().op: break;
	case spirv::OpBranchConditional(0).op: break;
	case spirv::OpSwitch(0).op: break;
	default: outCode.main->push(spirv::OpBranch(), 0, { id });
	}

	outCode.main->push(spirv::OpLabel(), id);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLabel)
{
	outCode.main->push(spirv::OpLabel(), outCode.toSpvID(i.index));

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpJump)
{
	outCode.main->push(spirv::OpBranch(), 0, { outCode.toSpvID(i.refs[0]) });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpJumpCond)
{
	auto cond = outCode.toSpvID(i.refs[0]);
	auto t = outCode.toSpvID(i.refs[1]);
	auto f = outCode.toSpvID(i.refs[2]);

	outCode.main->push(spirv::OpBranchConditional(0), 0, { cond, t, f });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLoop)//TODO
{

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpAssign)
{
	auto lhsVar = i.refs[0];
	auto rhsVal = i.refs[1];

	outCode.main->push(spirv::OpStore(0), 0, { outCode.toSpvID(lhsVar), outCode.toSpvID(rhsVal) });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpCompare)
{
	auto cllrOp = (Operator)i.operands[0];

	auto spvLhs = outCode.toSpvID(i.refs[0]);
	auto spvRhs = outCode.toSpvID(i.refs[1]);
	auto const& outType = inCode.getIns(i.outType);

	auto op = spirv::OpNop();

	if (outType.op == Opcode::TYPE_FLOAT)
	{
		switch (cllrOp)
		{
		case Operator::COMP_EQ: op = spirv::OpFOrdEqual(); break;
		case Operator::COMP_NEQ: op = spirv::OpFOrdNotEqual(); break;
		case Operator::COMP_GT: op = spirv::OpFOrdGreaterThan(); break;
		case Operator::COMP_GTE: op = spirv::OpFOrdGreaterThanEqual(); break;
		case Operator::COMP_LT: op = spirv::OpFOrdLessThan(); break;
		case Operator::COMP_LTE: op = spirv::OpFOrdLessThanEqual(); break;
		default: return;//TODO complain
		}

	}
	else if (outType.op == Opcode::TYPE_INT_SIGN)
	{
		switch (cllrOp)
		{
		case Operator::COMP_EQ: op = spirv::OpIEqual(); break;
		case Operator::COMP_NEQ: op = spirv::OpINotEqual(); break;
		case Operator::COMP_GT: op = spirv::OpSGreaterThan(); break;
		case Operator::COMP_GTE: op = spirv::OpSGreaterThanEqual(); break;
		case Operator::COMP_LT: op = spirv::OpSLessThan(); break;
		case Operator::COMP_LTE: op = spirv::OpSLessThanEqual(); break;
		default: return;//TODO complain
		}

	}
	else if (outType.op == Opcode::TYPE_INT_UNSIGN)
	{
		switch (cllrOp)
		{
		case Operator::COMP_EQ: op = spirv::OpIEqual(); break;
		case Operator::COMP_NEQ: op = spirv::OpINotEqual(); break;
		case Operator::COMP_GT: op = spirv::OpUGreaterThan(); break;
		case Operator::COMP_GTE: op = spirv::OpUGreaterThanEqual(); break;
		case Operator::COMP_LT: op = spirv::OpULessThan(); break;
		case Operator::COMP_LTE: op = spirv::OpULessThanEqual(); break;
		default: return;//TODO complain
		}

	}
	else if (outType.op == Opcode::TYPE_BOOL)
	{
		switch (cllrOp)
		{
		case Operator::LOGIC_AND: op = spirv::OpLogicalAnd(); break;
		case Operator::LOGIC_OR: op = spirv::OpLogicalOr(); break;
		default: return;//TODO complain
		}

	}
	else
	{
		//TODO complain
	}

	auto id = outCode.toSpvID(i.index);
	auto spvOutType = outCode.types.findOrMake(spirv::OpTypeBool(), {});

	outCode.main->pushTyped(op, spvOutType, id, { outCode.toSpvID(spvLhs), outCode.toSpvID(spvRhs) });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueCast)//TODO check impl
{
	outCode.main->pushTyped(spirv::OpBitcast(), outCode.toSpvID(i.outType), outCode.toSpvID(i.index), { outCode.toSpvID(i.refs[0]) });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueConstruct)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.outType);

	auto cllrArgs = cllr::Finder()
		.ops({ Opcode::CONSTRUCT_ARG })
		->setOffset(off + 1)
		->setLimit(i.operands[0])
		->find(inCode.getCode());

	auto args = cinq::map<Instruction, spirv::SSA>(cllrArgs, LAMBDA(in<Instruction> i) { return outCode.toSpvID(i.refs[0]); });

	outCode.main->pushTyped(spirv::OpCompositeConstruct((uint32_t)args.size()), t, id, args);
	//TODO consider calling constructor here. Or inCode the CLLR. idk.
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpConstructArg) {}//search-ahead

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueDeref)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.outType);

	//TODO check memory operands
	outCode.main->pushTyped(spirv::OpLoad(0), t, id, { outCode.toSpvID(i.refs[0]) });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExpand)
{
	auto id = outCode.toSpvID(i.index);

	auto inID = outCode.toSpvID(i.refs[0]);
	auto t = inCode.getIns(i.outType);

	auto spvOp = spirv::OpNop();

	if (t.op == cllr::Opcode::TYPE_INT_SIGN)
	{
		spvOp = spirv::OpSConvert();
	}
	else if (t.op == cllr::Opcode::TYPE_INT_UNSIGN)
	{
		spvOp = spirv::OpUConvert();
	}
	else if (t.op == cllr::Opcode::TYPE_FLOAT)
	{
		spvOp = spirv::OpFConvert();
	}
	else
	{
		//TODO complain
		return;
	}

	outCode.main->pushTyped(spvOp, outCode.toSpvID(t.index), id, { inID });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExpr)
{
	auto id = outCode.toSpvID(i.index);
	auto const& outType = inCode.getIns(i.outType);
	auto spvOutType = outCode.toSpvID(outType.index);

	auto cllrOp = (Operator)i.operands[0];

	auto spvLhs = outCode.toSpvID(i.refs[0]);
	auto spvRhs = outCode.toSpvID(i.refs[1]);

	if (cllrOp == Operator::POW)
	{
		auto lib = outCode.addImport("GLSL.std.450");

		outCode.main->pushTyped(spirv::OpExtInst(2), spvOutType, id, { lib, (uint32_t)spirv::GLSL450Ext::Pow, spvLhs, spvRhs });
		return;
	}

	auto exprFn = outCode.exprImpls.find(i.op);

	if (exprFn == outCode.exprImpls.end())
	{
		//TODO complain
		return;
	}

	auto op = exprFn->second(i, cllrOp, spvLhs, spvRhs, inCode, outCode);

	if (op == spirv::OpNop())
	{
		//TODO complain
		return;
	}

	outCode.main->pushTyped(op, spvOutType, id, { spvLhs, spvRhs });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExprUnary)
{
	auto id = outCode.toSpvID(i.index);
	auto spvOutType = outCode.toSpvID(i.outType);

	auto lhs = outCode.toSpvID(i.refs[0]);

	auto cllrOp = (Operator)i.operands[0];

	auto op = spirv::OpNop();

	auto outType = inCode.getIns(i.outType);

	if (cllrOp == Operator::ABS)
	{
		auto extOp = spirv::GLSL450Ext::None;

		if (outType.op == Opcode::TYPE_FLOAT)
		{
			extOp = spirv::GLSL450Ext::FAbs;
		}
		else if (outType.op == Opcode::TYPE_INT_SIGN)
		{
			extOp = spirv::GLSL450Ext::SAbs;
		}
		else
		{
			//TODO complain
			return;
		}

		auto lib = outCode.addImport("GLSL.std.450");

		outCode.main->pushTyped(spirv::OpExtInst(1), spvOutType, id, { lib, (uint32_t)extOp, lhs });
		return;
	}
	else if (outType.op == Opcode::TYPE_FLOAT)
	{
		if (cllrOp == Operator::NEG)
		{
			op = spirv::OpFNegate();

		}

	}
	else if (outType.op == Opcode::TYPE_INT_SIGN)
	{
		if (cllrOp == Operator::BIT_NEG)
		{
			op = spirv::OpNot();
		}
		else if (cllrOp == Operator::NEG)
		{
			op = spirv::OpSNegate();
		}

	}
	else if (outType.op == Opcode::TYPE_INT_UNSIGN)
	{
		if (cllrOp == Operator::BIT_NEG)
		{
			op = spirv::OpNot();
		}

	}
	else if (outType.op == Opcode::TYPE_BOOL)
	{
		if (cllrOp == Operator::BOOL_NOT)
		{
			op = spirv::OpLogicalNot();

		}

	}

	if (op == spirv::OpNop())
	{
		//TODO complain
		return;
	}

	outCode.main->pushTyped(op, spvOutType, id, { lhs });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInvokePos)
{
	//TODO adjust for non-compute shaders

	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.outType);

	auto u32 = outCode.types.findOrMake(spirv::OpTypeInt(), { 32, 0 });
	//auto i32 = outCode.types.findOrMake(spirv::OpTypeInt(), { 32, 1 });
	auto ptrIn = outCode.types.typeInPtr(u32);

	auto index = outCode.consts.findOrMake(u32, i.operands[0]);

	auto globalInvokeXYZ = outCode.builtins.getBuiltinVar(outCode.getCurrentEntry().type, spirv::BuiltIn::GlobalInvocationId);

	auto access = outCode.createSSA();

	outCode.main->pushTyped(spirv::OpAccessChain(1), ptrIn, access, { globalInvokeXYZ, index });
	outCode.main->pushTyped(spirv::OpLoad(0), u32, id, { access });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInvokeSize)
{
	//TODO I don't think this works for non-compute shaders

	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.outType);

	auto u32 = outCode.types.findOrMake(spirv::OpTypeInt(), { 32, 0 });
	auto ptrIn = outCode.types.typeInPtr(u32);

	auto index = outCode.consts.findOrMake(u32, i.operands[0]);

	auto globalInvokeXYZ = outCode.builtins.getBuiltinVar(outCode.getCurrentEntry().type, spirv::BuiltIn::GlobalSize);

	auto access = outCode.createSSA();

	outCode.main->pushTyped(spirv::OpAccessChain(1), ptrIn, access, { globalInvokeXYZ, index });
	outCode.main->pushTyped(spirv::OpLoad(0), u32, id, { access });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitArray)
{
	auto t = outCode.toSpvID(i.outType);

	spirv::Type typeData;
	if (!outCode.types.findData(t, typeData))
	{
		//TODO complain
		return;
	}

	spirv::SSA innerType = typeData.operands[0];
	uint32_t reqLength = typeData.operands[1];

	auto cllrElems = cllr::Finder()
		.ops({ Opcode::LIT_ARRAY_ELEM })
		->setOffset(off + 1)
		->setLimit(i.operands[0])
		->find(inCode.getCode());

	auto elems = cinq::map<Instruction, spirv::SSA>(cllrElems, LAMBDA(in<Instruction> i) { return outCode.toSpvID(i.refs[0]); });

	if (elems.size() < reqLength)
	{
		//TODO this might be confusing. MIGHT.
		spirv::SSA fillID = (elems.size() == 1) ? elems[0] : outCode.consts.findOrMakeNullFor(innerType);

		while (elems.size() < reqLength)
		{
			elems.push_back(fillID);
		}

	}

	bool isConst = true;

	for (auto e : elems)
	{
		//yes, even UNKNOWNs break the constant promise.
		if (outCode.getSection(e) != spirv::SpvSection::CONST)
		{
			isConst = false;
			break;
		}

	}

	spirv::SSA spvID = 0;

	if (isConst)
	{
		spvID = outCode.consts.findOrMakeComposite(t, elems);

	}
	else
	{
		spvID = outCode.createSSA();

		outCode.main->pushTyped(spirv::OpCompositeConstruct((uint32_t)elems.size()), t, spvID, elems);

	}

	outCode.setSpvSSA(i.index, spvID);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLitArrayElem) {} //search-ahead

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitBool)
{
	//Don't wory about find + making types inCode literals; it'll probably be made elsewhere

	auto outID = outCode.consts.findOrMake(outCode.toSpvID(i.outType), i.operands[0]);

	outCode.setSpvSSA(i.index, outID);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitFloat)
{
	auto str = inCode.getString(i.operands[0]);

	//TODO consider a better algorithm
	auto v = std::stof(str);

	auto outID = outCode.consts.findOrMakeFP32(outCode.toSpvID(i.outType), v);

	outCode.setSpvSSA(i.index, outID);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitInt)
{
	auto outID = outCode.consts.findOrMake(outCode.toSpvID(i.outType), i.operands[0], i.operands[1]);

	outCode.setSpvSSA(i.index, outID);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitStr)
{
	//TODO figure outCode strings within SPIR-V
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueMember)
{
	auto id = outCode.toSpvID(i.index);
	auto outT = outCode.toSpvID(i.outType);
	auto v = outCode.toSpvID(i.refs[0]);

	spirv::VarData vData;

	if (!outCode.main->getVarMeta(v, vData) && !outCode.gloVars->getVarMeta(v, vData))
	{
		//TODO complain
	}

	auto u32 = outCode.types.findOrMake(spirv::OpTypeInt(), { 32, 0 });
	auto c = outCode.consts.findOrMake(u32, i.operands[0]);

	auto ptrType = outCode.types.findOrMake(spirv::OpTypePointer(), { (uint32_t)vData.strClass, outT });

	auto access = outCode.createSSA();

	outCode.main->pushTyped(spirv::OpAccessChain(1), ptrType, access, { v, c });
	outCode.main->pushTyped(spirv::OpLoad(0), outT, id, { access });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueNull)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.outType);

	outCode.main->pushTyped(spirv::OpConstantNull(), t, id, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueReadVar)
{
	auto outVal = outCode.toSpvID(i.index);
	auto inVal = outCode.toSpvID(i.refs[0]);
	auto outType = outCode.toSpvID(i.outType);

	outCode.main->pushTyped(spirv::OpLoad(0), outType, outVal, { inVal });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueSample)
{
	auto outID = outCode.toSpvID(i.index);
	auto outType = outCode.toSpvID(i.outType);

	auto tex = outCode.toSpvID(i.refs[0]);
	auto uv = outCode.toSpvID(i.refs[1]);

	outCode.main->pushTyped(spirv::OpImageSampleImplicitLod(0), outType, outID, { tex, uv });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueSign)
{
	auto inValue = outCode.toSpvID(i.refs[0]);

	auto width = inCode.getIns(i.outType).operands[0];
	auto words = (width / 32);

	auto uint = outCode.types.findOrMake(spirv::OpTypeInt(), { width, 0 });
	auto sint = outCode.types.findOrMake(spirv::OpTypeInt(), { width, 1 });

	spirv::SSA bigInt = 0;

	if (words == 1)
	{
		bigInt = outCode.consts.findOrMake(uint, 0x7FFFFFFFu);
	}
	else if (words == 2)
	{
		bigInt = outCode.consts.findOrMake(uint, 0xFFFFFFFFu, 0x7FFFFFFFu);
	}
	else
	{
		std::vector<uint32_t> bigIntLits;
		bigIntLits.assign(words, 0xFFFFFFFF);

		bigIntLits[bigIntLits.size() - 1] = 0x7FFFFFFF;

		bigInt = outCode.createSSA();
		//TODO consider making the constant page able to use any width of constant (currently restricted to 64-bit)
		outCode.main->pushTyped(spirv::OpConstant((uint32_t)(bigIntLits.size() - 1)), uint, bigInt, bigIntLits);

	}

	auto tmp = outCode.createSSA();
	auto outValue = outCode.toSpvID(i.index);

	outCode.main->pushTyped(spirv::OpExtInst(2), uint, tmp, { outCode.addImport("GLSL.std.450"), (uint32_t)spirv::GLSL450Ext::UMin, inValue, bigInt });
	outCode.main->pushTyped(spirv::OpBitcast(), sint, outValue, { tmp });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueSubarray)
{
	auto id = outCode.toSpvID(i.index);
	auto outT = outCode.toSpvID(i.outType);
	auto v = outCode.toSpvID(i.refs[0]);

	auto index = outCode.toSpvID(i.refs[1]);
	auto index2D = outCode.toSpvID(i.refs[2]);

	spirv::VarData vData;

	if (!outCode.main->getVarMeta(v, vData) && !outCode.gloVars->getVarMeta(v, vData))
	{
		//TODO complain
	}

	auto ptrType = outCode.types.findOrMake(spirv::OpTypePointer(), { (uint32_t)vData.strClass, outT });

	auto access = outCode.createSSA();

	if (index2D != 0)
	{
		outCode.main->pushTyped(spirv::OpInBoundsAccessChain(1), ptrType, access, { v, index });
	}
	else
	{
		outCode.main->pushTyped(spirv::OpInBoundsAccessChain(2), ptrType, access, { v, index, index2D });
	}

	outCode.main->pushTyped(spirv::OpLoad(0), outT, id, { access });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueUnsign)
{
	auto width = inCode.getIns(i.outType).operands[0];

	auto inValue = outCode.toSpvID(i.refs[0]);

	auto uint = outCode.types.typeUInt(width);
	auto sint = outCode.types.typeInt(width);

	auto zero = outCode.consts.findOrMake(sint, 0);

	auto tmp = outCode.createSSA();
	auto outValue = outCode.toSpvID(i.index);

	outCode.main->pushTyped(spirv::OpExtInst(2), sint, tmp, { outCode.addImport("GLSL.std.450"), (uint32_t)spirv::GLSL450Ext::SMax, inValue, zero });
	outCode.main->pushTyped(spirv::OpBitcast(), uint, outValue, { tmp });

	/* I'm gonna leave the old version here for posterity's sake. And maybe use it if performance for an OpExtInst call is bad enough
	auto boolType = outCode.types.findOrMake(spirv::OpTypeBool());

	auto cmp = outCode.createSSA();
	auto ltLabel = outCode.createSSA();
	auto ltResult = outCode.createSSA();
	auto gteLabel = outCode.createSSA();
	auto gteResult = outCode.createSSA();
	auto endLabel = outCode.createSSA();
	auto outValue = outCode.toSpvID(i.index);

	//The code below should translate to:
	//if (x < 0) {
	//	  0
	//} else {
	//	  (uint32_t)x
	//}

	outCode.main->pushTyped(spirv::OpSLessThan(), boolType, cmp, { inValue, zero });
	outCode.main->push(spirv::OpBranchConditional(), 0, { cmp, ltLabel, gteLabel });
	outCode.main->push(spirv::OpLabel(), ltLabel);
	outCode.main->pushTyped(spirv::OpConstant(), uint, ltResult, { 0 }); //DO NOT USE THE CONSTANT SECTION FOR THIS
	outCode.main->push(spirv::OpBranch(), 0, { endLabel });
	outCode.main->push(spirv::OpLabel(), gteLabel);
	outCode.main->pushTyped(spirv::OpBitcast(), uint, gteResult, { inValue });
	outCode.main->push(spirv::OpBranch(), 0, { endLabel });
	outCode.main->push(spirv::OpLabel(), endLabel);
	outCode.main->pushTyped(spirv::OpPhi(4), uint, outValue, { ltResult, ltLabel, gteResult, gteLabel });
	*/
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueVecSwizzle)
{
	auto outVecLen = inCode.getIns(i.outType).operands[0];

	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.outType);
	auto inVec = outCode.toSpvID(i.refs[0]);

	std::vector<uint32_t> args = { inVec, inVec };
	for (size_t x = 0; x < outVecLen; ++x)
	{
		args.push_back(i.operands[x]);
	}

	outCode.main->pushTyped(spirv::OpVectorShuffle(outVecLen), t, id, args);

}

/*
Now, don't shoot me. DO NOT SHOOT ME.

but uh
OpConstantNull fulfills the Caliburn spec. Seemingly.
TODO: test against the spec. Worried about arrays, vectors.
*/
CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueZero)
{
	auto id = outCode.toSpvID(i.index);
	auto t = outCode.toSpvID(i.outType);

	outCode.main->push(spirv::OpConstantNull(), id, { t });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpReturn)
{
	outCode.main->push(spirv::OpReturn(), 0, {});

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpReturnValue)
{
	auto val = outCode.toSpvID(i.refs[0]);

	outCode.main->push(spirv::OpReturnValue(), 0, { val });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpDiscard)
{
	//outCode.main->push(spirv::OpTerminateInvocation(), 0, {});
	//I know it's deprecated, but like... eh.
	outCode.main->push(spirv::OpKill(), 0, {});

}

CLLR_EXPR_IMPL(cllr::spirv_impl::OpExprFloat)
{
	switch (op)
	{
	case Operator::ADD: return spirv::OpFAdd();
	case Operator::SUB: return spirv::OpFSub();
	case Operator::MUL: return spirv::OpFMul();
	case Operator::DIV: return spirv::OpFDiv();
	case Operator::MOD: return spirv::OpFMod();
	}

	return spirv::OpNop();
}

CLLR_EXPR_IMPL(cllr::spirv_impl::OpExprIntSign)
{
	switch (op)
	{
	case Operator::ADD: return spirv::OpIAdd();
	case Operator::SUB: return spirv::OpISub();
	case Operator::MUL: return spirv::OpIMul();
	case Operator::INTDIV: return spirv::OpSDiv();
	case Operator::MOD: return spirv::OpSMod();
	case Operator::BIT_AND: return spirv::OpBitwiseAnd();
	case Operator::BIT_OR: return spirv::OpBitwiseOr();
	case Operator::BIT_XOR: return spirv::OpBitwiseXor();
	}

	return spirv::OpNop();
}

CLLR_EXPR_IMPL(cllr::spirv_impl::OpExprIntUnsign)
{
	switch (op)
	{
	case Operator::ADD: return spirv::OpIAdd();
	case Operator::SUB: return spirv::OpISub();
	case Operator::MUL: return spirv::OpIMul();
	case Operator::INTDIV: return spirv::OpUDiv();
	case Operator::MOD: return spirv::OpUMod();
	case Operator::BIT_AND: return spirv::OpBitwiseAnd();
	case Operator::BIT_OR: return spirv::OpBitwiseOr();
	case Operator::BIT_XOR: return spirv::OpBitwiseXor();
	}

	return spirv::OpNop();
}

CLLR_EXPR_IMPL(cllr::spirv_impl::OpExprVector)
{
	return 0;
}

CLLR_EXPR_IMPL(cllr::spirv_impl::OpExprMatrix)
{
	return 0;
}
