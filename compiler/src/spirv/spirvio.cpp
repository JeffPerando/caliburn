
#include "spirv/spirvio.h"

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

//See https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)
SSA SpvIO::getBuiltinVar(ExecutionModel model, BuiltIn b)
{
	auto found = builtinIOs.find(b);

	if (found != builtinIOs.end())
	{
		return found->second;
	}

	auto& t = spvAsm->types;
	auto& d = *spvAsm->decs;

	spirv::SSA id = 0;

	auto i32 = t.typeInt();
	auto u32 = t.typeUInt();

	bool normalDecorate = true;

	switch (b)
	{
	case BuiltIn::Position: PASS;
	case BuiltIn::PointSize: PASS;
	case BuiltIn::ClipDistance: PASS; //TODO this seems to be used by frag shaders as well
	case BuiltIn::CullDistance: { //vertex
		//TODO this struct is used by multiple kinds of IO. It's also inside arrays.
		auto f32 = t.typeFP();
		auto v4_f32 = t.typeVec(4, f32);

		/*
		out gl_PerVertex {
			vec4 gl_Position;
			float gl_PointSize;
			float gl_ClipDistance[];
			float gl_ClipDistance[];
		};
		*/
		auto gl_PerVertex = t.typeStruct({ v4_f32, f32, f32, f32 }, { BuiltIn::Position, BuiltIn::PointSize, BuiltIn::ClipDistance, BuiltIn::CullDistance });

		id = makeOutVar(gl_PerVertex);
		normalDecorate = false;

	}; break;
	case BuiltIn::VertexId: { //vertex
		id = makeInVar(i32);
	}; break;
	case BuiltIn::InstanceId: { //vertex
		id = makeInVar(i32);
	}; break;
	case BuiltIn::PrimitiveId: { //tess control
		//Note to future self: This is a barrier from making SPIR-V and CLLR assemblers contain multiple shaders.
		StorageClass sc = (model == ExecutionModel::Geometry ? StorageClass::Output : StorageClass::Input);
		id = makeVar(t.typePtr(i32, sc), sc);
	}; break;
	case BuiltIn::InvocationId: { //tess control
		id = makeInVar(i32);
	}; break;
	case BuiltIn::Layer: { //geometry
		id = makeOutVar(i32);
	}; break;
	case BuiltIn::ViewportIndex: { //geometry
		id = makeOutVar(i32);
	}; break;
	case BuiltIn::TessLevelOuter: { //tess control
		auto f32 = t.typeFP();
		auto fa4 = t.typeArray(4, f32);

		id = makeOutVar(fa4);
	}; break;
	case BuiltIn::TessLevelInner: { //tess control
		auto f32 = t.typeFP();
		auto fa2 = t.typeArray(2, f32);

		id = makeOutVar(fa2);
	}; break;
	case BuiltIn::TessCoord: { //tess eval
		auto v3 = t.typeVec(3);
		id = makeInVar(t.typeInPtr(v3));
	}; break;
	case BuiltIn::PatchVertices: { //tess control
		id = makeInVar(i32);
	}; break;
	case BuiltIn::FragCoord: { //frag
		auto v4 = t.typeVec(4);
		id = makeInVar(v4);
	}; break;
	case BuiltIn::PointCoord: { //frag
		auto v2 = t.typeVec(2);
		id = makeInVar(v2);
	}; break;
	case BuiltIn::FrontFacing: { //frag
		auto boolType = t.typeBool();
		id = makeInVar(boolType);
	}; break;
	case BuiltIn::SampleId: { //frag
		id = makeInVar(i32);
	}; break;
	case BuiltIn::SamplePosition: { //frag
		auto v2 = t.typeVec(2);
		id = makeInVar(v2);
	}; break;
	case BuiltIn::SampleMask: {}; break; //FIXME this seems to be a runtime array.
	case BuiltIn::FragDepth: { //frag
		id = makeOutVar(t.typeFP());
	}; break;
	case BuiltIn::HelperInvocation: {}; break;
	case BuiltIn::NumWorkgroups: { //GLCompute
		id = makeInVar(t.typeVec(3, u32));
	}; break;
	case BuiltIn::WorkgroupSize: { //GLCompute
		//FIXME this is a compile-time constant, not a variable
		id = makeInVar(t.typeVec(3, u32));
	}; break;
	case BuiltIn::WorkgroupId: { //GLCompute
		id = makeInVar(t.typeVec(3, u32));
	}; break;
	case BuiltIn::LocalInvocationId: { //GLCompute
		id = makeInVar(t.typeVec(3, u32));
	}; break;
	case BuiltIn::GlobalInvocationId: { //GLCompute
		id = makeInVar(t.typeVec(3, u32));
	}; break;
	case BuiltIn::LocalInvocationIndex: { //GLCompute
		id = makeInVar(u32);
	}; break;
	case BuiltIn::WorkDim: {}; break;
	case BuiltIn::GlobalSize: {}; break;
	case BuiltIn::EnqueuedWorkgroupSize: {}; break;
	case BuiltIn::GlobalOffset: {}; break;
	case BuiltIn::GlobalLinearId: {}; break;
	case BuiltIn::SubgroupSize: {}; break;
	case BuiltIn::SubgroupMaxSize: {}; break;
	case BuiltIn::NumSubgroups: {}; break;
	case BuiltIn::NumEnqueuedSubgroups: {}; break;
	//see https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
	case BuiltIn::SubgroupId: {
		id = makeInVar(u32);
	}; break;
	case BuiltIn::SubgroupLocalInvocationId: {
		id = makeInVar(u32);
	}; break;
	case BuiltIn::VertexIndex: { //vertex
		//see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VertexIndex.html
		id = makeInVar(u32);
	}; break;
	case BuiltIn::InstanceIndex: { //vertex
		//see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/InstanceIndex.html
		id = makeInVar(u32);
	}; break;
	//see https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
	case BuiltIn::SubgroupEqMask: {
		id = makeInVar(t.typeVec(4, u32));
	}; break;
	case BuiltIn::SubgroupGeMask: {
		id = makeInVar(t.typeVec(4, u32));
	}; break;
	case BuiltIn::SubgroupGtMask: {
		id = makeInVar(t.typeVec(4, u32));
	}; break;
	case BuiltIn::SubgroupLeMask: {
		id = makeInVar(t.typeVec(4, u32));
	}; break;
	case BuiltIn::SubgroupLtMask: {
		id = makeInVar(t.typeVec(4, u32));
	}; break;
	case BuiltIn::BaseVertex: { //vertex
		//see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/BaseVertex.html
		id = makeInVar(u32);
	}; break;
	case BuiltIn::BaseInstance: { //vertex
		//see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/BaseInstance.html
		id = makeInVar(u32);
	}; break;
	case BuiltIn::DrawIndex: { //vertex
		//see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/DrawIndex.html
		id = makeInVar(u32);
	}; break;
	case BuiltIn::PrimitiveShadingRateKHR: { //vertex/geometry(??????)
		//see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/PrimitiveShadingRateKHR.html
		id = makeOutVar(u32);
		/*
		The variable decorated with PrimitiveShadingRateKHR within the MeshEXT Execution Model must also be decorated with the PerPrimitiveEXT decoration

		see also https://github.com/KhronosGroup/Vulkan-Docs/blob/main/proposals/VK_EXT_mesh_shader.adoc

		Can't find the decoration in the official SPIR-V documentation so I'll just
		*/
		//FIXME
	}; break;
	default: break;//TODO complain
	}

	builtinIOs.emplace(b, id);

	if (normalDecorate)
	{
		d.decorateBuiltIn(id, b);
	}

	return id;
}

SSA SpvIO::getOutputFor(ExecutionModel type, out<SSA> outType, uint32_t loc)
{
	auto found = shaderOuts.find(type);

	if (found != shaderOuts.end())
	{
		return found->second;
	}

	auto& code = *spvAsm->main.get();
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
