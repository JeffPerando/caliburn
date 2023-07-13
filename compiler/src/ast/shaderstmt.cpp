
#include "ast/shaderstmt.h"

#include "cllr/cllrasm.h"
#include "cllr/cllropt.h"

#include "spirv/cllrspirv.h"

#include "types/cbrn_std.h"

using namespace caliburn;

uptr<Shader> ShaderStageStatement::compile(sptr<SymbolTable> table, ptr<std::vector<uint32_t>> cbir, OptimizeLevel lvl)
{
	auto codeAsm = cllr::Assembler(type);

	code->resolveSymbols(table, codeAsm);

	code->emitDeclCLLR(table, codeAsm);

	//TODO emit CBIR
	/*
	if (cbir)
	{
		code->emitCBIR(cbir);
	}
	*/

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
