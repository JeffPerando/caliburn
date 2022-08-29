
#pragma once

#include <map>
#include <string>
#include <vector>

namespace caliburn
{
	enum class ShaderType
	{
		COMPUTE,
		VERTEX,
		FRAGMENT,
		TESS_CTRL,
		TESS_EVAL,
		GEOMETRY,
		RT_GEN,
		RT_CLOSE,
		RT_ANY_HIT,
		RT_INTERSECT,
		RT_MISS,
		TASK,
		MESH
	};

	struct DescriptorSet
	{
		std::string name;
		uint32_t binding;
		uint32_t type;

	};

	struct VertexInputAttribute
	{
		std::string name;
		uint32_t location;
		uint32_t format;
		
	};

	struct Shader
	{
		std::string name;
		ShaderType type;
		const uint32_t* spirv;
		size_t spirv_length;
		std::vector<VertexInputAttribute> inputs;
		std::vector<DescriptorSet> sets;

	};

	class Compiler
	{
	private:
		uint32_t optimizeLvl = 2;
		std::map<std::string, std::string> dynTypes;
	public:
		Compiler() {}

		/*
		Sets the optimization level for the compiler. Follows normal C++ nomenclature:

		O0 (oh zero): no optimizations.
		O1: only quick to perform optimizations.
		O2: gets you 95% of the performance.
		O3: takes ages to compile, but ensures maximum performance.

		Each level increases compilation times, but promises better peformance.

		Giving the compiler a higher number will not make your code run faster.
		*/
		Compiler* o(uint32_t opLvl);

		/*
		In Caliburn, a type can be defined as "dynamic", which means the actual type is determined
		during the actual compilation process. This is where they're defined. This allows devs to
		write one shader that can use both, say, FP16 and FP32.
		*/
		Compiler* setDynamicType(std::string inner, std::string concrete);

		/*
		Parses source code text to CBIR. CBIR is merely the binary representation of a Caliburn
		source file, and is the format recommended for redistribution.

		This method does NOT imply endorsement of the redistribution of shader source code. Please
		package your shaders as CBIR for redistribution.
		*/
		std::vector<uint32_t>* parseSrc(std::string text);

		/*
		Compiles CBIR into a final set of shaders. Each shader is in SPIR-V format, and contains
		all the metadata needed to interface with Vulkan.

		cbir: Contains the bytecode used to construct the shader.
		shaderName: The name of the shader object to compile.
		shaderDest: The final vector for the final Vulkan shaders.

		Returns a boolean; True for a successful compilation, and false for critical errors.

		An empty shaderName will result in a failed compilation.
		*/
		bool compile(std::vector<uint32_t>* cbir, std::string shaderName, std::vector<Shader>& shaderDest);

	};

}
