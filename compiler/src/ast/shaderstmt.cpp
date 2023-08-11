
#include "ast/shaderstmt.h"

#include "cllr/cllrasm.h"
#include "cllr/cllropt.h"
#include "cllr/cllrvalid.h"

#include "spirv/cllrspirv.h"

#include "types/cbrn_std.h"

using namespace caliburn;

uptr<Shader> ShaderStage::compile(sptr<SymbolTable> table, sptr<const CompilerSettings> settings, ref<std::vector<sptr<Error>>> allErrs)
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

	auto stageID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::SHADER_STAGE, { (uint32_t)type, nameID }, {}));

	base->code->declareSymbols(stageTable);
	base->code->emitDeclCLLR(stageTable, codeAsm);

	codeAsm.push(cllr::Instruction(cllr::Opcode::SHADER_STAGE_END, {}, { stageID }));

	auto valid = cllr::Validator(settings);

	if (!valid.validate(codeAsm.getCode()))
	{
		valid.errors->dump(allErrs);
	}

	auto op = cllr::Optimizer(settings);
	op.optimize(codeAsm);

	auto spirvAsm = cllr::SPIRVOutAssembler();

	auto out = new_uptr<Shader>(type, spirvAsm.translateCLLR(codeAsm));
	
	spirvAsm.errors->dump(allErrs);

	if (type == ShaderType::VERTEX)
	{
		for (auto const& [name, index] : codeAsm.getInputs())
		{
			out->inputs.push_back(VertexInputAttribute{ name, index });

		}

	}

	return out;
}

std::vector<uptr<Shader>> ShaderStatement::compile(sptr<SymbolTable> table, sptr<CompilerSettings> settings, ref<std::vector<sptr<Error>>> compileErrs)
{
	std::vector<uptr<Shader>> shaders;

	auto shaderSyms = new_sptr<SymbolTable>(table);

	for (auto const& io : ioVars)
	{
		shaderSyms->add(io->name, io);
	}

	for (auto const& stage : stages)
	{
		auto shader = stage->compile(shaderSyms, settings, compileErrs);

		uint32_t d = 0;

		for (auto const& desc : descriptors)
		{
			shader->sets.push_back(DescriptorSet{ desc.second->str, d++ });

		}

		shaders.push_back(std::move(shader));

	}

	return shaders;
}
