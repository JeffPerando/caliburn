
#include "spirv/spirvio.h"

#include <algorithm>

#include "spirv/spirvtype.h"
#include "spirv/cllrspirv.h"

using namespace caliburn::spirv;

SSA SpvIO::makeVar(SSA type, StorageClass sc)
{
	auto id = spvAsm->createSSA();
	spvAsm->gloVars->pushVar(type, id, sc, 0);
	return id;
}

SSA SpvIO::makeInVar(SSA type)
{
	return makeVar(spvAsm->types.typeInPtr(type), spirv::StorageClass::Input);
}

SSA SpvIO::makeOutVar(SSA type)
{
	return makeVar(spvAsm->types.typeOutPtr(type), spirv::StorageClass::Output);
}

//See "https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)"
SSA SpvIO::getBuiltinVar(ExecutionModel model, BuiltIn b)
{
	auto found = builtinIOs.find(b);

	if (found != builtinIOs.end())
	{
		return found->second;
	}

	auto& t = spvAsm->types;
	auto& d = *spvAsm->decs;

	BuiltinVar data;

	bool normalDecorate = true;

	switch (b)
	{
	case BuiltIn::Position: PASS;
	case BuiltIn::PointSize: PASS;
	case BuiltIn::ClipDistance: PASS;
	case BuiltIn::CullDistance: {
		auto f32 = t.typeFP(32);
		auto v4_f32 = t.typeVec(4, f32);
		auto arr_f32 = t.typeRunArray(f32);

		/*
		out gl_PerVertex {
			vec4 gl_Position;
			float gl_PointSize;
			float gl_ClipDistance[];
			float gl_CullDistance[];
		};
		*/
		auto gl_PerVertex = t.typeStruct({ v4_f32, f32, arr_f32, arr_f32 }, { BuiltIn::Position, BuiltIn::PointSize, BuiltIn::ClipDistance, BuiltIn::CullDistance });

		data = BuiltinVar{
			{ BuiltIn::Position, BuiltIn::PointSize, BuiltIn::ClipDistance, BuiltIn::CullDistance },
			model == ExecutionModel::Fragment ? StorageClass::Input : StorageClass::Output,
			gl_PerVertex,
			{ ExecutionModel::Vertex,
			ExecutionModel::TessellationControl, ExecutionModel::TessellationEvaluation,
			ExecutionModel::Geometry, ExecutionModel::Fragment },
			{ Capability::Shader, Capability::ClipDistance, Capability::CullDistance }
		};

		normalDecorate = false;

	}; break;
	case BuiltIn::VertexId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::Vertex },
			{ Capability::Shader }
		};
	}; break;
	case BuiltIn::InstanceId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::Vertex },
			{ Capability::Shader }
		};
	}; break;
	case BuiltIn::PrimitiveId: {
		data = BuiltinVar{
			{ b },
			model == ExecutionModel::Geometry ? StorageClass::Output : StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::TessellationControl, ExecutionModel::TessellationEvaluation,
			ExecutionModel::Geometry, ExecutionModel::Fragment },
			{ Capability::Geometry, Capability::Tessellation,
			Capability::RayTracingKHR, Capability::MeshShadingNV }
		};
	}; break;
	case BuiltIn::InvocationId: {
		data = BuiltinVar{
			{ b },
			model == ExecutionModel::Fragment ? StorageClass::Input : StorageClass::Output,
			t.typeUInt(32),
			{ ExecutionModel::Geometry, ExecutionModel::TessellationControl },
			{ Capability::Geometry, Capability::Tessellation }
		};
	}; break;
	case BuiltIn::Layer: {
		data = BuiltinVar{
			{ b },
			model == ExecutionModel::Fragment ? StorageClass::Input : StorageClass::Output,
			t.typeUInt(32),
			{ ExecutionModel::Vertex,
			ExecutionModel::TessellationControl, ExecutionModel:: TessellationEvaluation,
			ExecutionModel::Geometry, ExecutionModel::Fragment },
			{ Capability::Geometry, Capability::ShaderLayer,
			Capability::ShaderViewportIndexLayerEXT, Capability::MeshShadingNV }
		};
	}; break;
	case BuiltIn::ViewportIndex: {
		data = BuiltinVar{
			{ b },
			model == ExecutionModel::Fragment ? StorageClass::Input : StorageClass::Output,
			t.typeUInt(32),
			{ ExecutionModel::Vertex,
			ExecutionModel::TessellationControl, ExecutionModel::TessellationEvaluation,
			ExecutionModel::Geometry, ExecutionModel::Fragment },
			{ Capability::MultiViewport, Capability::ShaderViewportIndex,
			Capability::ShaderViewportIndexLayerEXT, Capability::MeshShadingNV }
		};
	}; break;
	case BuiltIn::TessLevelOuter: {
		data = BuiltinVar{
			{ b },
			StorageClass::Output,
			t.typeArray(4, t.typeFP(32)),
			{ ExecutionModel::TessellationControl },
			{ Capability::Tessellation }
		};
	}; break;
	case BuiltIn::TessLevelInner: {
		data = BuiltinVar{
			{ b },
			StorageClass::Output,
			t.typeArray(2, t.typeFP(32)),
			{ ExecutionModel::TessellationControl },
			{ Capability::Tessellation }
		};
	}; break;
	case BuiltIn::TessCoord: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeFP(32)),
			{ ExecutionModel::TessellationEvaluation },
			{ Capability::Tessellation }
		};
	}; break;
	case BuiltIn::PatchVertices: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::TessellationControl },
			{ Capability::Tessellation }
		};
	}; break;
	case BuiltIn::FragCoord: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(4, t.typeFP(32)),
			{ ExecutionModel::Fragment },
			{ Capability::Shader }
		};
	}; break;
	case BuiltIn::PointCoord: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(2, t.typeFP(32)),
			{ ExecutionModel::Fragment },
			{ Capability::Shader }
		};
	}; break;
	case BuiltIn::FrontFacing: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeBool(),
			{ ExecutionModel::Fragment },
			{ Capability::Shader }
		};
	}; break;
	case BuiltIn::SampleId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::Fragment },
			{ Capability::SampleRateShading }
		};
	}; break;
	case BuiltIn::SamplePosition: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(2, t.typeFP(32)),
			{ ExecutionModel::Fragment },
			{ Capability::SampleRateShading }
		};
	}; break;
	case BuiltIn::SampleMask: {
		data = BuiltinVar{
			{ b },
			StorageClass::Output,
			t.typeRunArray(t.typeUInt(32)),
			{ ExecutionModel::Fragment },
			{ Capability::Shader }
		};
	}; break;
	case BuiltIn::FragDepth: {
		data = BuiltinVar{
			{ b },
			StorageClass::Output,
			t.typeFP(32),
			{ ExecutionModel::Fragment },
			{ Capability::Shader }
		};
	}; break;
	case BuiltIn::HelperInvocation: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeBool(),
			{ ExecutionModel::Fragment },
			{ Capability::Shader }
		};
	}; break;
	//Below is Kernel stuff
	//see https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_Env.html#_built_in_variables
	case BuiltIn::NumWorkgroups: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{}
		};
	}; break;
	case BuiltIn::WorkgroupSize: {
		//FIXME this is a compile-time constant, not a variable
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{}
		};
	}; break;
	case BuiltIn::WorkgroupId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{}
		};
	}; break;
	case BuiltIn::LocalInvocationId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{}
		};
	}; break;
	case BuiltIn::GlobalInvocationId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{}
		};
	}; break;
	case BuiltIn::LocalInvocationIndex: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{}
		};
	}; break;
	case BuiltIn::WorkDim: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel }
		};
	}; break;
	case BuiltIn::GlobalSize: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel }
		};
	}; break;
	case BuiltIn::EnqueuedWorkgroupSize: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel }
		};
	}; break;
	case BuiltIn::GlobalOffset: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(3, t.typeSizeT()),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel }
		};
	}; break;
	case BuiltIn::GlobalLinearId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeSizeT(),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel }
		};
	}; break;
	case BuiltIn::SubgroupSize: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel, Capability::GroupNonUniform, Capability::SubgroupBallotKHR }
		};
	}; break;
	case BuiltIn::SubgroupMaxSize: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel }
		};
	}; break;
	case BuiltIn::NumSubgroups: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel, Capability::GroupNonUniform }
		};
	}; break;
	case BuiltIn::NumEnqueuedSubgroups: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel }
		};
	}; break;
	//see https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
	case BuiltIn::SubgroupId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel, Capability::GroupNonUniform }
		};
	}; break;
	case BuiltIn::SubgroupLocalInvocationId: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::GLCompute, ExecutionModel::Kernel },
			{ Capability::Kernel, Capability::GroupNonUniform, Capability::SubgroupBallotKHR }
		};
	}; break;
	case BuiltIn::VertexIndex: {
		//see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VertexIndex.html
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::Vertex },
			{ Capability::Shader }
		};
	}; break;
	case BuiltIn::InstanceIndex: {
		//see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/InstanceIndex.html
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::Vertex },
			{ Capability::Shader }
		};
	}; break;
	//see https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
	case BuiltIn::SubgroupEqMask: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(4, t.typeUInt(32)),
			{},
			{ Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot }
		};
	}; break;
	case BuiltIn::SubgroupGeMask: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(4, t.typeUInt(32)),
			{},
			{ Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot }
		};
	}; break;
	case BuiltIn::SubgroupGtMask: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(4, t.typeUInt(32)),
			{},
			{ Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot }
		};
	}; break;
	case BuiltIn::SubgroupLeMask: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(4, t.typeUInt(32)),
			{},
			{ Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot }
		};
	}; break;
	case BuiltIn::SubgroupLtMask: {
		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeVec(4, t.typeUInt(32)),
			{},
			{ Capability::SubgroupBallotKHR, Capability::GroupNonUniformBallot }
		};
	}; break;
	case BuiltIn::BaseVertex: {
		//see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/BaseVertex.html

		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::Vertex },
			{ Capability::DrawParameters }
		};
	}; break;
	case BuiltIn::BaseInstance: {
		//see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/BaseInstance.html

		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::Vertex },
			{ Capability::DrawParameters }
		};
	}; break;
	case BuiltIn::DrawIndex: {
		//see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/DrawIndex.html

		data = BuiltinVar{
			{ b },
			StorageClass::Input,
			t.typeUInt(32),
			{ ExecutionModel::Vertex, ExecutionModel::TaskEXT, ExecutionModel::MeshEXT },
			{ Capability::DrawParameters, Capability::MeshShadingNV }
		};
	}; break;
	case BuiltIn::PrimitiveShadingRateKHR: {
		//see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/PrimitiveShadingRateKHR.html

		data = BuiltinVar{
			{ b },
			StorageClass::Output,
			t.typeUInt(32),
			{ ExecutionModel::Vertex, ExecutionModel::Geometry, ExecutionModel::MeshEXT },
			{ Capability::FragmentShadingRateKHR }
		};
		/*
		The variable decorated with PrimitiveShadingRateKHR within the MeshEXT Execution Model must also be decorated with the PerPrimitiveEXT decoration

		see also https://github.com/KhronosGroup/Vulkan-Docs/blob/main/proposals/VK_EXT_mesh_shader.adoc

		Can't find the decoration in the official SPIR-V documentation so I'll just
		*/
		//FIXME
	}; break;
	default: {
		throw std::runtime_error("Unknown builtin SPIR-V variable");
	}
	}

	//TODO use data.caps

	if (!data.execs.empty() && !std::binary_search(data.execs.begin(), data.execs.end(), model))
	{
		throw std::runtime_error("Builtin SPIR-V variable used with inappropriate shader type");
	}

	auto id = makeVar(data.typeID, data.kind);

	//One builtin can have multiple associated builtins; So we add this ID to all of them
	//This really only applies to the per-vertex builtin
	for (auto bn : data.builtins)
	{
		builtinIOs.emplace(bn, id);
	}

	if (normalDecorate)
	{
		d.decorateBuiltIn(id, b);
	}

	return id;
}

//TODO finish
SSA SpvIO::getOutputFor(ExecutionModel type, out<SSA> outType, uint32_t loc)
{
	if (auto found = shaderOuts.find(type); found != shaderOuts.end())
	{
		return found->second;
	}

	auto& code = *spvAsm->main;
	auto& t = spvAsm->types;
	auto& d = spvAsm->decs;

	SSA outID = 0;

	if (type == ExecutionModel::Vertex)
	{
		outType = t.typeVec(4);
		outID = getBuiltinVar(type, BuiltIn::Position);

	}
	else if (type == ExecutionModel::Fragment)
	{
		outType = t.typeVec(4);
		outID = makeOutVar(t.typeOutPtr(outType));

		d->decorate(outID, Decoration::Location, { loc });

	}

	shaderOuts.emplace(type, outID);

	return outID;
}
