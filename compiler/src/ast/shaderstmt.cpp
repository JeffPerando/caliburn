
#include "ast/shaderstmt.h"

#include "cllr/cllrasm.h"
#include "cllr/cllropt.h"
#include "cllr/cllrvalid.h"

#include "spirv/cllrspirv.h"

using namespace caliburn;

uptr<Shader> ShaderStage::compile(sptr<SymbolTable> table, sptr<const CompilerSettings> settings, out<std::vector<sptr<Error>>> allErrs)
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

	auto codeAsm = cllr::Assembler(type, settings);

	auto fullName = parentName->str + "_" + SHADER_TYPE_NAMES.at(type);
	auto nameID = codeAsm.addString(fullName);

	auto stageID = codeAsm.beginSect(cllr::Instruction(cllr::Opcode::SHADER_STAGE, { (uint32_t)type, nameID }, {}));

	base->code->declareSymbols(stageTable);
	base->code->emitCodeCLLR(stageTable, codeAsm);

	codeAsm.endSect(cllr::Instruction(cllr::Opcode::SHADER_STAGE_END, {}, { stageID }));

	auto validator = cllr::Validator(settings);
	//std::chrono::high_resolution_clock clock{};

	//auto startTime = clock.now();
	bool valid = validator.validate(codeAsm);
	//auto time = clock.now() - startTime;

	//std::cout << "Validation took " << (time.count() * 0.000001f) << " ms\n";

	if (!valid)
	{
		validator.errors->dump(allErrs);
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

		spirvAsm.errors->dump(allErrs);

	}
	else
	{
		//TODO complain
	}
	
	if (type == ShaderType::VERTEX)
	{
		for (auto const& [name, index] : codeAsm.getInputs())
		{
			//TODO assign input format
			outShader->inputs.push_back(VertexInputAttribute{ name, index, 0 });

		}

	}

	return outShader;
}

void ShaderStmt::compile(sptr<SymbolTable> table, sptr<CompilerSettings> settings, out<std::vector<uptr<Shader>>> shaders, out<std::vector<sptr<Error>>> compileErrs)
{
	auto shaderSyms = new_sptr<SymbolTable>(table);

	for (auto const& io : ioVars)
	{
		shaderSyms->add(io->name, io);
	}

	for (auto const& stage : stages)
	{
		auto shader = stage->compile(shaderSyms, settings, compileErrs);

		if (shader == nullptr)
		{
			continue;
		}

		uint32_t d = 0;

		for (auto const& desc : descriptors)
		{
			shader->sets.push_back(DescriptorSet{ desc.second->str, d++ });

		}

		shaders.push_back(std::move(shader));

	}

}
