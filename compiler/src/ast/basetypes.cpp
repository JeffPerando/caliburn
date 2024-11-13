
#include "ast/basetypes.h"

using namespace caliburn;

void TypeFloat::initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto selfType = new_sptr<ParsedType>(canonName);

	impl->ctors.add(new_sptr<BuiltinCtor>(
		selfType,
		std::vector{
			FnArg{selfType, "in"}
		},
		LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
		{
			return args[0];
		}
	));

}

void TypeInt::initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto selfType = new_sptr<ParsedType>(canonName);

	impl->ctors.add(new_sptr<BuiltinCtor>(
		selfType,
		std::vector{
			FnArg{selfType, "in"}
		},
		LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
		{
			return args[0];
		}
	));

}

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
	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_ARRAY, {}, { elemType->id, length.value }));

	initLowImpl(impl, gArgs, table, codeAsm);

	return impl;
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

	variants.insert(std::pair(gArgs, impl));

	initLowImpl(impl, gArgs, genTable, codeAsm);

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

	initLowImpl(impl, gArgs, genTable, codeAsm);

	return impl;
}

void TypeTexture::initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto pixel = gArgs->getType(0);
	auto pixImpl = pixel->resolve(table, codeAsm);

	impl->addMemberFn(new_sptr<BuiltinMethod>(
		new_sptr<ParsedType>(canonName, gArgs),
		table,
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

	auto unit = gArgs->getType(0);
	auto inner = unit->resolve(table, codeAsm);
	auto impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_VECTOR, { elements }, { inner->id }));

	variants.insert(std::pair(gArgs, impl));

	initLowImpl(impl, gArgs, table, codeAsm);

	return impl;
}

void TypeVector::initLowImpl(sptr<cllr::LowType> impl, sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto selfType = new_sptr<ParsedType>(canonName, gArgs);
	auto unit = gArgs->getType(0);
	auto inner = unit->resolve(table, codeAsm);

	/*
	vec2<T>(T x, T y)
	vec3<T>(T x, T y, T z)
	vec4<T>(T x, T y, T z, T w)
	*/

	std::vector<FnArg> defCtorArgs = {
		{unit, "x"},
		{unit, "y"},
		{unit, "z"},
		{unit, "w"}
	};

	impl->ctors.add(new_sptr<BuiltinCtor>(
		selfType,
		std::vector(defCtorArgs.begin(), defCtorArgs.begin() + elements),
		LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
		{
			cllr::OpArray cArgs = {};
			for (uint32_t i = 0; i < elements; ++i)
			{
				cArgs[i] = args[i].value;
			}

			return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, cArgs), outType);
		}
	));

	/*
	vec2<T>(vec2<T> in)
	vec3<T>(vec3<T> in)
	vec4<T>(vec4<T> in)
	*/

	impl->ctors.add(new_sptr<BuiltinCtor>(
		selfType,
		std::vector{
			FnArg{selfType, "in"}
		},
		LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
		{
			return args[0];
		}
	));

	if (elements == 3)
	{
		//vec3<T>(vec2<T> xy, T z)
		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			std::vector{
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "xy"},
				FnArg{unit, "z"}
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[0].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[0].value }), inner).value;

				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { v1_x, v1_y, args[1].value }), outType);
			}
		));

		//vec3<T>(T x, vec2<T> yz)
		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			std::vector{
				FnArg{unit, "x"},
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "yz"}
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
		//vec4<T>(vec2<T> xy, vec2<T> zw)
		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			std::vector{
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "xy"},
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "zw"},
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

		//vec4<T>(vec2<T> xy, T z, T w)
		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			std::vector{
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "xy"},
				FnArg{unit, "z"},
				FnArg{unit, "w"},
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[0].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[0].value }), inner).value;

				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { v1_x, v1_y, args[1].value, args[2].value }), outType);
			}
		));

		//vec4<T>(T x, T y, vec2<T> zw)
		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			std::vector{
				FnArg{unit, "x"},
				FnArg{unit, "y"},
				FnArg{new_sptr<ParsedType>("vec2", gArgs), "zw"}
			},
			LAMBDA(sptr<const SymbolTable> tbl, out<cllr::Assembler> cllrAsm, in<std::vector<cllr::TypedSSA>> args, sptr<cllr::LowType> outType)
			{
				auto v1_x = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 0 }, { args[2].value }), inner).value;
				auto v1_y = cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { 1 }, { args[2].value }), inner).value;

				return cllrAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_CONSTRUCT, {}, { args[0].value, args[1].value, v1_x, v1_y }), outType);
			}
		));

		//vec4<T>(vec3<T> xyz, T w)
		impl->ctors.add(new_sptr<BuiltinCtor>(
			selfType,
			std::vector{
				FnArg{new_sptr<ParsedType>("vec3", gArgs), "xyz"},
				FnArg{unit, "w"},
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

}
