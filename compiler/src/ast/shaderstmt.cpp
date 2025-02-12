
#include "ast/shaderstmt.h"

#include "cllr/cllrasm.h"
#include "cllr/cllropt.h"
#include "cllr/cllrtype.h"
#include "cllr/cllrvalid.h"

#include "spirv/cllrspirv.h"

using namespace caliburn;

uptr<Shader> ShaderStage::compile(sptr<const CompilerSettings> settings, out<std::vector<std::string>> errs, in<TextDoc> doc, sptr<SymbolTable> table, std::vector<IOVar> inputs, out<std::vector<IOVar>> outputs)
{
	sptr<SymbolTable> stageTable = table;

	if (!vtxInputs.empty())
	{
		stageTable = new_sptr<SymbolTable>(table);

		for (auto const& in : vtxInputs)
		{
			stageTable->add(in->name, in);

		}

	}

	auto codeAsm = cllr::Assembler(type, settings, inputs);

	auto const fullName = (std::stringstream() << parentName.str << "_" << SHADER_TYPE_NAMES.at(type)).str();
	auto const nameID = SCAST<uint32_t>(codeAsm.addString(fullName));

	auto const typeOut = base->returnType->resolve(stageTable, codeAsm);

	if (typeOut == nullptr)
	{
		codeAsm.errors->err("Could not resolve type", *base->returnType);

		return nullptr;
	}

	auto const stageID = codeAsm.beginSect(cllr::Instruction(cllr::Opcode::SHADER_STAGE, { (uint32_t)type, nameID }, { typeOut->id }).debug(first));

	base->code->emitCodeCLLR(stageTable, codeAsm);

	codeAsm.endSect(cllr::Instruction(cllr::Opcode::SHADER_STAGE_END, {}, { stageID }));

	if (!codeAsm.errors->empty())
	{
		codeAsm.errors->printout(errs, doc);

		return nullptr;
	}

	auto validator = cllr::Validator(settings);
	
	if (!validator.validate(codeAsm))
	{
		return nullptr;
	}

	auto op = cllr::Optimizer(settings);
	op.optimize(codeAsm);

	uptr<Shader> outShader;

	if (settings->gpuTarget == GPUTarget::SPIRV)
	{
		auto spirvAsm = cllr::SPIRVOutAssembler(settings);
		auto spirvCode = spirvAsm.translateCLLR(codeAsm);
		
		outShader = new_uptr<Shader>(type, spirvCode);

		spirvAsm.errors->printout(errs, doc);

	}
	else
	{
		//TODO complain
		return nullptr;
	}
	
	if (type == ShaderType::VERTEX)
	{
		for (auto const& var : codeAsm.getIOByType(ShaderIOVarType::INPUT))
		{
			//TODO assign input format
			outShader->inputs.push_back(VertexInputAttribute{ std::string(var.name), var.index, 0});

		}

	}

	outputs = codeAsm.getIOByType(ShaderIOVarType::OUTPUT);

	return outShader;
}

void ShaderStmt::compile(sptr<SymbolTable> table, sptr<CompilerSettings> settings, out<ShaderResult> result, in<TextDoc> doc)
{
	if (stages.empty())
	{
		return;
	}

	auto shaderSyms = new_sptr<SymbolTable>(table);

	for (auto const& io : ioVars)
	{
		shaderSyms->add(io->name, io);
	}

	std::sort(stages.begin(), stages.end());

	std::vector<IOVar> inputs{};

	for (auto const& stage : stages)
	{
		std::vector<IOVar> outputs;

		auto shader = stage->compile(settings, result.errors, doc, shaderSyms, inputs, outputs);

		if (shader == nullptr)
		{
			continue;
		}

		uint32_t descIdx = 0;

		for (auto const& [_, name] : descriptors)
		{
			shader->sets.push_back(DescriptorSet{ std::string(name.str), descIdx });
			++descIdx;
		}

		result.shaders.push_back(std::move(shader));

		for (auto const& out : outputs)
		{
			inputs.push_back(IOVar{ out.name, ShaderIOVarType::INPUT, out.index });

		}

	}

}
