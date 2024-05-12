
#include "ast/basetypes.h"

using namespace caliburn;

sptr<cllr::LowType> TypeArray::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (!sig->canApply(*gArgs))
	{
		//TODO complain
	}

	auto t = gArgs->getType(0);
	auto elemType = t->resolve(table, codeAsm);
	auto len = gArgs->getConst(1);
	auto lenRes = len->emitValueCLLR(table, codeAsm);
	cllr::TypedSSA length;

	MATCH(lenRes, cllr::TypedSSA, lenPtr)
	{
		length = *lenPtr;
	}
	else
	{
		codeAsm.errors->err("Invalid array length", *len);
	}

	//FIXME typecheck the length too .-.
	//FIXME I need to be able to extract an int constant from a Value
	//Technically I can just check the SSA ID for a constant, but that feels a lil hacky
	//Lowkey need to implement an interpreter anyway too...
	return codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_ARRAY, {}, { elemType->id, length.value }));
}

sptr<cllr::LowType> TypeStruct::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (auto found = variants.find(gArgs); found != variants.end())
	{
		return found->second;
	}

	auto genTable = new_sptr<SymbolTable>(table);

	//populate table with generics and members
	gArgs->apply(*genSig, genTable, codeAsm);

	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_STRUCT, { (uint32_t)members.size() }));

	for (auto& [name, type] : members)
	{
		if (auto rt = type->typeHint->resolve(genTable, codeAsm))
		{
			impl->addMember(name, rt);

		}
		else
		{
			//TODO complain
		}

	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::STRUCT_END, {}, { impl->id }));

	//avoid allocating the parsed type
	if (memberFns.empty())
	{
		return impl;
	}

	auto me = new_sptr<ParsedType>(canonName, gArgs);

	for (auto& fn : memberFns)
	{
		impl->addMemberFn(new_sptr<SrcMethod>(me, genTable, *fn));

	}

	return impl;
}

sptr<cllr::LowType> TypeTexture::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	auto genTable = new_sptr<SymbolTable>(table);

	gArgs->apply(genSig, genTable, codeAsm);

	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_TEXTURE, { SCAST<uint32_t>(kind) }));

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

sptr<cllr::LowType> TypeVector::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (!genSig->canApply(*gArgs))
	{
		//TODO complain
		return nullptr;
	}

	if (gArgs->empty())
	{
		gArgs = genSig->makeDefaultArgs();
	}

	if (auto found = variants.find(gArgs); found != variants.end())
	{
		return found->second;
	}

	auto inner = gArgs->getType(0)->resolve(table, codeAsm);
	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_VECTOR, { elements }, { inner->id }));

	variants.insert(std::pair(gArgs, impl));

	return impl;
}
