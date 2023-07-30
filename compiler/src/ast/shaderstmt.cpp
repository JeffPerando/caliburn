
#include "ast/shaderstmt.h"

#include "cllr/cllrasm.h"
#include "cllr/cllropt.h"
#include "cllr/cllrvalid.h"

#include "spirv/cllrspirv.h"

#include "types/cbrn_std.h"

using namespace caliburn;

uptr<Shader> ShaderStageStatement::compile(sptr<SymbolTable> table, sptr<const CompilerSettings> settings, ref<std::vector<sptr<Error>>> allErrs)
{
	auto codeAsm = cllr::Assembler(type, settings);

	code->emitDeclCLLR(table, codeAsm);

	if (settings->validate)
	{
		auto v = cllr::Validator(settings);

		if (!v.validate(codeAsm.getCode()))
		{
			v.errors->dump(allErrs);
			return nullptr;
		}

	}
	
	//TODO emit CBIR

	auto op = cllr::Optimizer(settings);
	op.optimize(codeAsm);

	auto spirvAsm = cllr::SPIRVOutAssembler();

	auto out = new_uptr<Shader>(type, spirvAsm.translateCLLR(codeAsm));

	spirvAsm.errors->dump(allErrs);

	//TODO add descriptor sets

	if (type == ShaderType::VERTEX)
	{
		for (auto const& ioVar : ios)
		{
			if (ioVar->getIOType() != ShaderIOVarType::INPUT)
			{
				continue;
			}

			out->inputs.push_back(VertexInputAttribute{ ioVar->name, ioVar->getIndex() });

		}

	}

	return out;
}
