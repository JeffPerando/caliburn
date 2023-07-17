
#include "ast/shaderstmt.h"

#include "cllr/cllrasm.h"
#include "cllr/cllropt.h"

#include "spirv/cllrspirv.h"

#include "types/cbrn_std.h"

using namespace caliburn;

uptr<Shader> ShaderStageStatement::compile(sptr<SymbolTable> table, OptimizeLevel lvl)
{
	auto codeAsm = cllr::Assembler(type);

	code->emitDeclCLLR(table, codeAsm);

	//TODO validate

	//TODO emit CBIR

	cllr::optimize(lvl, codeAsm);

	cllr::SPIRVOutAssembler spirvAsm;

	auto out = new_uptr<Shader>();

	out->type = type;
	out->spirv = spirvAsm.translateCLLR(codeAsm);

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
