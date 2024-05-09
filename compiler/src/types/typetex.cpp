
#include "types/typetex.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

sptr<cllr::LowType> TypeTex2D::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_TEXTURE, { SCAST<uint32_t>(TextureKind::_2D) }));
	
	impl->addMemberFn(new_sptr<BuiltinMethod>(
		new_sptr<ParsedType>(canonName),
		"sample",
		SCAST<uptr<GenericSignature>>(nullptr),
		std::vector<FnArg>({
			FnArg{new_sptr<ParsedType>("vec2"), "uv"}
		}),
		new_sptr<ParsedType>("vec4"),
		LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
		{
			return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_SAMPLE, {}, { args[0].value, args[1].value }), outType);
		}
	));
	
	return impl;
}
