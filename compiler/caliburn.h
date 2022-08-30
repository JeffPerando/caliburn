
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "langcore.h"

namespace caliburn
{
	/*
	Determines optimization level for compiler.

	Each level performs worse than the last, but in exchange gives the result
	better on-paper performance.
	*/
	enum class OptimizeLevel : uint32_t
	{
		//Equivalent to O0; no optimizations.
		NONE,
		//Equivalent to O1; only quick to perform optimizations.
		BASIC,
		//Equivalent to O2; gets you 95% of the performance.
		BALANCED,
		//Equivalent to O3; longest compile time and maximum performance.
		PERFORMANCE
	};

	class Compiler
	{
	private:
		OptimizeLevel optimizeLvl = OptimizeLevel::BALANCED;
		std::map<std::string, std::string> dynTypes;
	public:
		Compiler() {}

		/*
		Sets the optimization level for the compiler. See OptimizeLevel enum.
		*/
		Compiler* o(OptimizeLevel lvl);

		/*
		In Caliburn, a type can be defined as "dynamic", which means the actual
		type is determined during the actual compilation process. This is where
		they're defined. This allows devs to write one shader that can use both,
		say, FP16 and FP32.

		- inner must correlate to a dynamic type within a shader being compiled.
		- concrete must correlate to an existing type within a compiled shader,
		  and ideally be a standard type.
		*/
		Compiler* setDynamicType(std::string inner, std::string concrete);

		/*
		Parses source code text to CBIR. CBIR is merely the binary
		representation of a Caliburn source file, and is the format recommended
		for redistribution.

		This method does NOT imply endorsement of the redistribution of shader
		source code. Please package your shaders as CBIR for redistribution.
		*/
		std::vector<uint32_t>* parseSrc(std::string text);

		/*
		Compiles CBIR into a final set of shaders. Each shader is in SPIR-V
		format, and contains all the metadata needed to interface with Vulkan.

		cbir: Contains the bytecode used to construct the shader.
		shaderName: The name of the shader object to compile.
		shaderDest: The final vector for the final Vulkan shaders.

		Returns a boolean; True for a successful compilation, and false for
		critical errors.

		An empty shaderName will result in a failed compilation.
		*/
		bool compile(std::vector<uint32_t>* cbir, std::string shaderName,
			std::vector<Shader>& shaderDest);

	};

}
