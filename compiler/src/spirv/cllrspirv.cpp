
#include "spirv/cllrspirv.h"

#include "cinq.h"
#include "langcore.h"
#include "syntax.h"

using namespace caliburn;

cllr::SPIRVOutAssembler::SPIRVOutAssembler() : OutAssembler(Target::GPU)
{
	//here we go...

	impls[(uint32_t)Opcode::SHADER_STAGE] = spirv_impl::OpShaderStage;
	impls[(uint32_t)Opcode::SHADER_STAGE_END] = spirv_impl::OpShaderStageEnd;

	impls[(uint32_t)Opcode::FUNCTION] = spirv_impl::OpFunction;
	impls[(uint32_t)Opcode::VAR_FUNC_ARG] = spirv_impl::OpVarFuncArg;
	impls[(uint32_t)Opcode::FUNCTION_END] = spirv_impl::OpFunctionEnd;

	impls[(uint32_t)Opcode::VAR_LOCAL] = spirv_impl::OpVarLocal;
	impls[(uint32_t)Opcode::VAR_GLOBAL] = spirv_impl::OpVarGlobal;
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
	impls[(uint32_t)Opcode::VALUE_CONSTRUCT] = spirv_impl::OpValueConstruct;
	impls[(uint32_t)Opcode::CONSTRUCT_ARG] = spirv_impl::OpConstructArg;
	impls[(uint32_t)Opcode::VALUE_DEREF] = spirv_impl::OpValueDeref;
	impls[(uint32_t)Opcode::VALUE_EXPR] = spirv_impl::OpValueExpr;
	impls[(uint32_t)Opcode::VALUE_EXPR_UNARY] = spirv_impl::OpValueExprUnary;
	impls[(uint32_t)Opcode::VALUE_INVOKE_POS] = spirv_impl::OpValueInvokePos;
	impls[(uint32_t)Opcode::VALUE_INVOKE_SIZE] = spirv_impl::OpValueInvokeSize;
	impls[(uint32_t)Opcode::VALUE_LIT_ARRAY] = spirv_impl::OpValueLitArray;
	impls[(uint32_t)Opcode::LIT_ARRAY_ELEM] = spirv_impl::OpLitArrayElem;
	impls[(uint32_t)Opcode::VALUE_LIT_BOOL] = spirv_impl::OpValueLitBool;
	impls[(uint32_t)Opcode::VALUE_LIT_FP] = spirv_impl::OpValueLitFloat;
	impls[(uint32_t)Opcode::VALUE_LIT_INT] = spirv_impl::OpValueLitInt;
	impls[(uint32_t)Opcode::VALUE_LIT_STR] = spirv_impl::OpValueLitStr;
	impls[(uint32_t)Opcode::VALUE_MEMBER] = spirv_impl::OpValueMember;
	impls[(uint32_t)Opcode::VALUE_NULL] = spirv_impl::OpValueNull;
	impls[(uint32_t)Opcode::VALUE_READ_VAR] = spirv_impl::OpValueReadVar;
	impls[(uint32_t)Opcode::VALUE_SUBARRAY] = spirv_impl::OpValueSubarray;
	impls[(uint32_t)Opcode::VALUE_ZERO] = spirv_impl::OpValueZero;

	impls[(uint32_t)Opcode::RETURN] = spirv_impl::OpReturn;
	impls[(uint32_t)Opcode::RETURN_VALUE] = spirv_impl::OpReturnValue;
	impls[(uint32_t)Opcode::DISCARD] = spirv_impl::OpDiscard;

}

uptr<std::vector<uint32_t>> cllr::SPIRVOutAssembler::translateCLLR(ref<cllr::Assembler> cllrAsm, ref<std::vector<sptr<cllr::Instruction>>> code)
{
	for (size_t off = 0; off < code.size(); ++off)
	{
		auto const& i = code[off];

		auto fn = (impls[(uint32_t)i->op]);
		(*fn)(*i, off, cllrAsm, *this);

	}

	types.dump(*spvTypes);
	consts.dump(*spvConsts);

	//Magic numbers
	spvHeader->pushRaw({ spirv::SPIRV_FILE_MAGIC_NUMBER , spirv::Version(1, 5), spirv::CALIBURN_GENERATOR_MAGIC_NUMBER, maxSSA(), 0 });

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

void cllr::SPIRVOutAssembler::searchAheadCLLR(
	ref<std::vector<spirv::SSA>> outIDs,
	std::vector<cllr::Opcode> ops,
	size_t off,
	uint32_t count,
	ref<cllr::Assembler> cllrAsm,
	std::function<cllr::SSA(sptr<cllr::Instruction>)> filter)
{
	outIDs.reserve(count);

	cllr::InstructionVec cllrIns;
	cllrAsm.findAll(cllrIns, ops, off, count);

	for (auto const& i : cllrIns)
	{
		outIDs.push_back(toSpvID(filter(i)));
	}

}

spirv::SSA cllr::SPIRVOutAssembler::createSSA()
{
	auto entry = spirv::SSAEntry{ nextSSA, spirv::OpNop() };

	++nextSSA;

	ssaEntries.push_back(entry);
	ssaToSection.push_back(spirv::SSASection::UNKNOWN);

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

spirv::SSASection cllr::SPIRVOutAssembler::getSection(spirv::SSA id)
{
	if (id >= ssaToSection.size())
		return spirv::SSASection::UNKNOWN;

	return ssaToSection[id];
}

void cllr::SPIRVOutAssembler::setSection(spirv::SSA id, spirv::SSASection sec)
{
	if (id == 0)
		return;//TODO complain

	if (id >= ssaToSection.size())
		return;//TODO complain

	if (ssaToSection[id] != spirv::SSASection::UNKNOWN)
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
	auto type = (ShaderType)i.operands[0];
	auto ex = SHADER_EXECUTION_MODELS.find(type)->second;

	std::vector<spirv::SSA> ios;
	out.searchAheadCLLR(ios, { Opcode::VAR_SHADER_IN, Opcode::VAR_SHADER_OUT }, off + 1, i.operands[1], in, lambda(auto i) {
		return i->index;
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

	std::vector<uint32_t> fnArgs;
	out.searchAheadCLLR(fnArgs, { Opcode::VAR_FUNC_ARG }, off + 1, i.operands[0], in, lambda(auto i) {
		return i->refs[0];
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

	std::vector<uint32_t> fnArgs;
	out.searchAheadCLLR(fnArgs, { Opcode::CALL_ARG }, off + 1, i.operands[0], in, lambda(auto i) {
		return i->refs[0];
	});

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

	//It's tempting to replace this with actually implementing OpStructMember
	//BUT, I might change the spec to make adding new members less fidgety.
	std::vector<uint32_t> members;
	out.searchAheadCLLR(members, { Opcode::STRUCT_MEMBER }, off + 1, i.operands[0], in, lambda(auto i) {
		return i->refs[0];
	});

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

	out.main->push(spirv::OpBranchConditional(0), 0, { cond, t, f });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLoop)//TODO
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
	auto id = out.toSpvID(i.index);
	auto spvOutType = out.types.findOrMake(spirv::OpTypeBool(), {});

	auto cllrOp = (Operator)i.operands[0];

	auto op = spirv::OpNop();

	auto outType = in.codeFor(i.outType);

	if (outType->op == Opcode::TYPE_FLOAT)
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
	else if (outType->op == Opcode::TYPE_INT)
	{
		//Unsigned int
		if (outType->operands[1] == 0)
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
		else //Signed int
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

	}
	else if (outType->op == Opcode::TYPE_BOOL)
	{
		switch (cllrOp)
		{
		case Operator::AND: op = spirv::OpLogicalAnd(); break;
		case Operator::OR: op = spirv::OpLogicalOr(); break;
		default: return;//TODO complain
		}

	}
	else
	{
		//TODO complain
	}

	auto lhs = out.toSpvID(i.refs[0]);
	auto rhs = out.toSpvID(i.refs[1]);

	out.main->pushTyped(op, spvOutType, id, { lhs, rhs } );

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueCast)//TODO check impl
{
	out.main->pushTyped(spirv::OpBitcast(), out.toSpvID(i.outType), out.toSpvID(i.index), { out.toSpvID(i.refs[0]) });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueConstruct)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.outType);

	std::vector<uint32_t> args;
	out.searchAheadCLLR(args, { Opcode::CONSTRUCT_ARG }, off + 1, i.operands[0], in, lambda(auto i) { return i->refs[0]; });

	out.main->pushTyped(spirv::OpCompositeConstruct((uint32_t)args.size()), t, id, args);
	//TODO consider calling constructor here. Or in the CLLR. idk.
}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpConstructArg) {}//search-ahead

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueDeref)
{
	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.outType);

	//TODO check memory operands
	out.main->pushTyped(spirv::OpLoad(), t, id, { out.toSpvID(i.refs[0]) });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExpr)
{
	auto id = out.toSpvID(i.index);
	auto spvOutType = out.toSpvID(i.outType);

	auto lhs = out.toSpvID(i.refs[0]);
	auto rhs = out.toSpvID(i.refs[1]);

	auto cllrOp = (Operator)i.operands[0];

	if (cllrOp == Operator::POW)
	{
		auto lib = out.addImport("GLSL.std.450");

		out.main->pushTyped(spirv::OpExtInst(2), spvOutType, id, { lib, (uint32_t)spirv::GLSL450Ext::Pow, lhs, rhs });
		return;
	}

	auto op = spirv::OpNop();

	auto outType = in.codeFor(i.outType);

	if (outType->op == Opcode::TYPE_FLOAT)
	{
		switch (cllrOp)
		{
		case Operator::ADD: op = spirv::OpFAdd(); break;
		case Operator::SUB: op = spirv::OpFSub(); break;
		case Operator::MUL: op = spirv::OpFMul(); break;
		case Operator::DIV: op = spirv::OpFDiv(); break;
		case Operator::INTDIV: return; //TODO complain
		case Operator::MOD: op = spirv::OpFMod(); break;
		default: return;//TODO complain
		}

	}
	else if (outType->op == Opcode::TYPE_INT)
	{
		switch (cllrOp)
		{
		case Operator::ADD: op = spirv::OpIAdd(); break;
		case Operator::SUB: op = spirv::OpISub(); break;
		case Operator::MUL: op = spirv::OpIMul(); break;
		case Operator::BIT_AND: op = spirv::OpBitwiseAnd(); break;
		case Operator::BIT_OR: op = spirv::OpBitwiseOr(); break;
		case Operator::BIT_XOR: op = spirv::OpBitwiseXor(); break;
		}

		//Unsigned int
		if (outType->operands[1] == 0)
		{
			switch (cllrOp)
			{
			case Operator::DIV: return; //TODO complain
			case Operator::INTDIV: op = spirv::OpUDiv(); break;
			case Operator::MOD: op = spirv::OpUMod(); break;
			default: return;//TODO complain
			}

		}
		else //Signed int
		{
			switch (cllrOp)
			{
			case Operator::DIV: return; //TODO complain
			case Operator::INTDIV: op = spirv::OpSDiv(); break;
			case Operator::MOD: op = spirv::OpSMod(); break;
			default: return;//TODO complain
			}

		}

	}
	else
	{
		//TODO complain
		return;
	}

	out.main->pushTyped(op, spvOutType, id, { lhs, rhs });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueExprUnary)
{
	auto id = out.toSpvID(i.index);
	auto spvOutType = out.toSpvID(i.outType);

	auto lhs = out.toSpvID(i.refs[0]);
	
	auto cllrOp = (Operator)i.operands[0];

	auto op = spirv::OpNop();

	auto outType = in.codeFor(i.outType);
	
	if (cllrOp == Operator::ABS)
	{
		auto extOp = spirv::GLSL450Ext::None;

		if (outType->op == Opcode::TYPE_FLOAT)
		{
			extOp = spirv::GLSL450Ext::FAbs;
		}
		else if (outType->op == Opcode::TYPE_INT)
		{
			extOp = spirv::GLSL450Ext::SAbs;
		}
		else
		{
			//TODO complain
			return;
		}

		auto lib = out.addImport("GLSL.std.450");

		out.main->pushTyped(spirv::OpExtInst(1), spvOutType, id, { lib, (uint32_t)extOp, lhs });
		return;
	}
	else if (cllrOp == Operator::BIT_NEG)
	{
		if (outType->op == Opcode::TYPE_FLOAT)
		{
			//TODO complain
			return;
		}
		
		op = spirv::OpNot();

	}
	else if (outType->op == Opcode::TYPE_FLOAT)
	{
		if (cllrOp == Operator::NEG)
		{
			op = spirv::OpFNegate();

		}
		else
		{
			//TODO complain
			return;
		}

	}
	else if (outType->op == Opcode::TYPE_INT)
	{
		if (cllrOp == Operator::BIT_NEG)
		{
			op = spirv::OpNot();
		}
		else if (outType->operands[1] == 0) //Unsigned int
		{
			if (cllrOp == Operator::NEG)
			{
				op = spirv::OpSNegate();
			}
			else if (cllrOp == Operator::SIGN)
			{
				//TODO
				return;
			}
			else
			{
				//TODO complain
				return;
			}

		}
		else //Signed int
		{
			if (cllrOp == Operator::UNSIGN)
			{
				//TODO
				return;
			}
			else
			{
				//TODO complain
				return;
			}
			
		}

	}
	else if (outType->op == Opcode::TYPE_BOOL)
	{
		if (cllrOp == Operator::BOOL_NOT)
		{
			op = spirv::OpLogicalNot();

		}
		else
		{
			//TODO complain
			return;
		}

	}
	else
	{
		//TODO complain
		return;
	}

	out.main->pushTyped(op, spvOutType, id, { lhs });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInvokePos)
{
	//TODO adjust for non-compute shaders

	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.outType);

	auto globalInvokeXYZ = out.builtins.getBuiltinVar(spirv::BuiltIn::GlobalInvocationId);

	auto u32 = out.types.findOrMake(spirv::OpTypeInt(), { 32, 0 });
	auto i32 = out.types.findOrMake(spirv::OpTypeInt(), { 32, 1 });
	auto index = out.consts.findOrMake(i32, i.operands[0]);

	auto ptrIn = out.types.findOrMake(spirv::OpTypePointer(), { (uint32_t)spirv::StorageClass::Input, u32 });

	auto access = out.createSSA();

	out.main->pushTyped(spirv::OpAccessChain(1), ptrIn, access, { globalInvokeXYZ, index });
	out.main->pushTyped(spirv::OpLoad(), u32, id, { access });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueInvokeSize)
{
	//TODO I don't think this works in non-compute shaders

	auto id = out.toSpvID(i.index);
	auto t = out.toSpvID(i.outType);

	auto globalInvokeXYZ = out.builtins.getBuiltinVar(spirv::BuiltIn::GlobalSize);

	auto u32 = out.types.findOrMake(spirv::OpTypeInt(), { 32, 0 });
	auto i32 = out.types.findOrMake(spirv::OpTypeInt(), { 32, 1 });
	auto index = out.consts.findOrMake(i32, i.operands[0]);

	auto ptrIn = out.types.findOrMake(spirv::OpTypePointer(), { (uint32_t)spirv::StorageClass::Input, u32 });

	auto access = out.createSSA();

	out.main->pushTyped(spirv::OpAccessChain(1), ptrIn, access, { globalInvokeXYZ, index });
	out.main->pushTyped(spirv::OpLoad(), u32, id, { access });

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpValueLitArray)
{
	auto t = out.toSpvID(i.outType);

	spirv::Type typeData;
	if (!out.types.findData(t, typeData))
	{
		//TODO complain
		return;
	}

	spirv::SSA innerType = typeData.operands[0];
	uint32_t reqLength = typeData.operands[1];

	//TODO write system for searching CLLR instructions
	
	std::vector<uint32_t> elems;
	out.searchAheadCLLR(elems, { Opcode::LIT_ARRAY_ELEM }, off + 1, i.operands[0], in, lambda(auto i) { return i->refs[0]; });

	if (elems.size() < reqLength)
	{
		//TODO this might be confusing. MIGHT.
		spirv::SSA fillID = (elems.size() == 1) ? elems[0] : out.consts.findOrMakeNullFor(innerType);

		while (elems.size() < reqLength)
		{
			elems.push_back(fillID);
		}

	}

	bool isConst = true;

	for (auto e : elems)
	{
		//yes, even UNKNOWNs break the constant promise.
		if (out.getSection(e) != spirv::SSASection::CONST)
		{
			isConst = false;
			break;
		}

	}

	spirv::SSA spvID = 0;

	if (isConst)
	{
		spvID = out.consts.findOrMakeComposite(t, elems);

	}
	else
	{
		spvID = out.createSSA();

		out.main->pushTyped(spirv::OpCompositeConstruct((uint32_t)elems.size()), t, spvID, elems);

	}
	
	out.setSpvSSA(i.index, spvID);

}

CLLR_SPIRV_IMPL(cllr::spirv_impl::OpLitArrayElem) {} //search-ahead

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
	auto id = out.toSpvID(i.index);
	auto outT = out.toSpvID(i.outType);
	auto v = out.toSpvID(i.refs[0]);

	spirv::VarData vData;

	if (!out.main->findVarMeta(v, vData) && !out.gloVars->findVarMeta(v, vData))
	{
		//TODO complain
	}

	auto i32 = out.types.findOrMake(spirv::OpTypeInt(), { 32, 1 });
	auto c = out.consts.findOrMake(i32, i.operands[0]);

	auto ptrType = out.types.findOrMake(spirv::OpTypePointer(), { (uint32_t)vData.strClass, outT });

	auto access = out.createSSA();

	out.main->pushTyped(spirv::OpAccessChain(1), ptrType, access, { v, c });
	out.main->pushTyped(spirv::OpLoad(), outT, id, { access });

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
	auto id = out.toSpvID(i.index);
	auto outT = out.toSpvID(i.outType);
	auto v = out.toSpvID(i.refs[0]);

	auto index = out.toSpvID(i.refs[1]);
	auto index2D = out.toSpvID(i.refs[2]);

	spirv::VarData vData;

	if (!out.main->findVarMeta(v, vData) && !out.gloVars->findVarMeta(v, vData))
	{
		//TODO complain
	}

	auto ptrType = out.types.findOrMake(spirv::OpTypePointer(), { (uint32_t)vData.strClass, outT });

	auto access = out.createSSA();

	if (index2D != 0)
	{
		out.main->pushTyped(spirv::OpInBoundsAccessChain(1), ptrType, access, { v, index });
	}
	else
	{
		out.main->pushTyped(spirv::OpInBoundsAccessChain(2), ptrType, access, { v, index, index2D });
	}

	out.main->pushTyped(spirv::OpLoad(), outT, id, { access });

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
