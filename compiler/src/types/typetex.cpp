
#include "types/typetex.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

sptr<cllr::LowType> TypeTexture::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_TEXTURE, { SCAST<uint32_t>(kind) }));
	
	auto genTable = new_sptr<SymbolTable>(table);

	gArgs->apply(genSig, genTable, codeAsm);

	impl->addMemberFn(new_sptr<BuiltinMethod>(
		new_sptr<ParsedType>(canonName, gArgs), genTable,
		"sample",
		SCAST<uptr<GenericSignature>>(nullptr),
		std::vector<FnArg>({
			FnArg{new_sptr<ParsedType>(TEX_SAMPLE_VECS.at(kind).data()), "uv"}
		}),
		new_sptr<ParsedType>("Pixel"),
		LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
		{
			return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_SAMPLE, {}, { args[0].value, args[1].value }), outType);
		}
	));
	
	return impl;
}
