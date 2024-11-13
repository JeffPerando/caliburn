
#include "ast/basetypes.h"

using namespace caliburn;

sptr<cllr::LowType> TypeArray::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (!sig.canApply(*gArgs))
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

	auto pixel = gArgs->getType(0);
	auto pixImpl = pixel->resolve(genTable, codeAsm);

	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_TEXTURE, { SCAST<uint32_t>(kind) }, { pixImpl->id }));

	impl->addMemberFn(new_sptr<BuiltinMethod>(
		new_sptr<ParsedType>(canonName, gArgs), genTable,
		"sample",
		SCAST<uptr<GenericSignature>>(nullptr),
		std::vector<FnArg>({
			FnArg{new_sptr<ParsedType>(TEX_SAMPLE_VECS.at(kind).data()), "uv"}
		}),
		pixel,
		LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
		{
			return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_SAMPLE, {}, { args[0].value, args[1].value }), outType);
		}
	));

	return impl;
}

sptr<cllr::LowType> TypeVector::resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (!genSig.canApply(*gArgs))
	{
		//TODO complain
		return nullptr;
	}

	if (gArgs->empty())
	{
		gArgs = genSig.makeDefaultArgs();
	}

	if (auto found = variants.find(gArgs); found != variants.end())
	{
		return found->second;
	}

	auto genTable = new_sptr<SymbolTable>(table);

	gArgs->apply(genSig, genTable, codeAsm);

	auto selfType = new_sptr<ParsedType>(canonName, gArgs);
	auto unit = gArgs->getType(0);
	auto inner = unit->resolve(table, codeAsm);
	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_VECTOR, { elements }, { inner->id }));

	variants.insert(std::pair(gArgs, impl));

	std::vector<FnArg> defCtorArgs = {
		{unit, "x"},
		{unit, "y"},
		{unit, "z"},
		{unit, "w"}
	};
	
	impl->ctors.add(new_sptr<BuiltinCtor>(
		selfType,
		genTable,
		std::vector(defCtorArgs.begin(), defCtorArgs.begin() + elements),
		LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
		{
			cllr::OpArray cArgs = {};
			for (auto i = 0; i < elements; ++i)
			{
				cArgs[i] = args[i].value;
			}

			return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, cArgs), outType);
		}
	));

	//TODO copy constructor

	if (elements == 3)
	{
		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			genTable,
			std::vector{
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "v1"},
				FnArg{unit, "zIn"}
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[0].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[0].value }), inner).value;

				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { v1_x, v1_y, args[1].value }), outType);
			}
		));

		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			genTable,
			std::vector{
				FnArg{unit, "xIn"},
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "v1"}
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[1].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[1].value }), inner).value;

				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { args[0].value, v1_x, v1_y }), outType);
			}
		));

	}

	if (elements == 4)
	{
		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			genTable,
			std::vector{
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "v1"},
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "v2"},
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[0].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[0].value }), inner).value;
				auto v2_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[1].value }), inner).value;
				auto v2_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[1].value }), inner).value;
				
				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { v1_x, v1_y, v2_x, v2_y }), outType);
			}
		));

		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			genTable,
			std::vector{
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "v1"},
				FnArg{unit, "inZ"},
				FnArg{unit, "inW"},
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[0].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[0].value }), inner).value;

				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { v1_x, v1_y, args[1].value, args[2].value }), outType);
			}
		));

		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			genTable,
			std::vector{
				FnArg{unit, "inX"},
				FnArg{unit, "inY"},
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "v1"}
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[2].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[2].value }), inner).value;

				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { args[0].value, args[1].value, v1_x, v1_y }), outType);
			}
		));

		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			genTable,
			std::vector{
				FnArg{new_sptr<ParsedType>("vec3", gArgs), "v1"},
				FnArg{unit, "inW"},
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[0].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[0].value }), inner).value;
				auto v1_z = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 2 }, { args[0].value }), inner).value;

				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { v1_x, v1_y, v1_z, args[1].value }), outType);
			}
		));

	}
	
	return impl;
}
