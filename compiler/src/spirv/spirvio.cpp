
#include "spirv/spirvio.h"

#include "spirv/cllrspirv.h"

using namespace caliburn;

spirv::SSA spirv::SpvIO::typeInt32()
{
	return spvAsm->types.findOrMake(spirv::OpTypeInt(), { 32, 1 });
}

spirv::SSA spirv::SpvIO::typeUInt32()
{
	return spvAsm->types.findOrMake(spirv::OpTypeInt(), { 32, 0 });
}

spirv::SSA spirv::SpvIO::typeFP32()
{
	return spvAsm->types.findOrMake(spirv::OpTypeFloat(), { 32 });
}

spirv::SSA spirv::SpvIO::typeVec(uint32_t len, spirv::SSA inner)
{
	return spvAsm->types.findOrMake(spirv::OpTypeVector(), { inner, len });
}

spirv::SSA spirv::SpvIO::typeArray(uint32_t len, spirv::SSA inner)
{
	return spvAsm->types.findOrMake(spirv::OpTypeArray(), { inner, len });
}

spirv::SSA spirv::SpvIO::typeStruct(std::vector<uint32_t> members, std::vector<spirv::BuiltIn> decs)
{
	auto id = spvAsm->types.findOrMake(spirv::OpTypeStruct((uint32_t)members.size()), members);

	if (!decs.empty())
	{
		for (uint32_t index = 0; index < decs.size(); ++index)
		{
			spvAsm->decs->pushRaw({spirv::OpMemberDecorate(), id, index, (uint32_t)spirv::Decoration::BuiltIn, (uint32_t)decs[index]});
		}

	}

	return id;
}

spirv::SSA spirv::SpvIO::typePtr(spirv::SSA inner, spirv::StorageClass sc)
{
	return spvAsm->types.findOrMake(spirv::OpTypePointer(), { (uint32_t)sc, inner });
}

spirv::SSA spirv::SpvIO::makeVar(spirv::SSA type, spirv::StorageClass sc)
{
	auto id = spvAsm->createSSA();
	spvAsm->main->push(spirv::OpVariable(), id, { type, (uint32_t)sc });
	return id;
}

//See https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)
spirv::SSA spirv::SpvIO::getBuiltinVar(spirv::BuiltIn builtin)
{
	auto found = builtinIOs.find(builtin);

	if (found != builtinIOs.end())
	{
		return found->second;
	}

	spirv::SSA type = 0;
	spirv::StorageClass sClass = spirv::StorageClass::UniformConstant;//idk man

	switch (builtin)
	{
	case BuiltIn::VertexIndex: {
		type = typeInt32();
		sClass = spirv::StorageClass::Input;
	}
	default: break;//TODO complain
	}

	auto id = makeVar(type, sClass);
	spvAsm->decs->pushRaw({ spirv::OpDecorate(1), id, (uint32_t)Decoration::BuiltIn, (uint32_t)builtin });

	builtinIOs.emplace(builtin, id);

	return id;
}

spirv::SSA spirv::SpvIO::getOutputFor(spirv::ExecutionModel type, ref<spirv::SSA> outType, uint32_t loc)
{
	auto found = shaderOuts.find(type);

	if (found != shaderOuts.end())
	{
		return found->second;
	}

	spirv::SSA outID = 0;

	if (type == ExecutionModel::Vertex)
	{
		auto& code = *spvAsm->main.get();
		auto& types = spvAsm->types;

		auto fp = typeFP32();
		auto v4_fp = typeVec(4, fp);

		auto i32 = typeInt32();
		auto a1_32 = typeArray(1, i32);

		/*
		out gl_PerVertex
		{
			vec4 gl_Position;
			float gl_PointSize;
			int gl_ClipDistance[];
			int gl_ClipDistance[];
		};
		*/

		auto glPerVertex = typeStruct(
			{ v4_fp,				fp,					a1_32,					a1_32 },
			{ BuiltIn::Position,	BuiltIn::PointSize, BuiltIn::ClipDistance,	BuiltIn::CullDistance }
		);

		spvAsm->decs->pushRaw({ spirv::OpDecorate(1), glPerVertex, (uint32_t)Decoration::Block });

		outType = v4_fp;
		outID = makeVar(typePtr(glPerVertex, StorageClass::Output), StorageClass::Output);

	}
	else if (type == ExecutionModel::Fragment)
	{
		outType = typeVec(4, typeFP32());
		outID = makeVar(typePtr(outType, StorageClass::Output), StorageClass::Output);

		spvAsm->decs->pushRaw({ spirv::OpDecorate(2), outID, (uint32_t)Decoration::Location, loc });

	}

	shaderOuts.emplace(type, outID);

	return outID;
}
