
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "langcore.h"

namespace caliburn
{
	struct Statement;

	/*
	Enum denoting how much to optimize emitted shader code.

	Each level should emit more optimal code, obviously.

	Regarding whether to add a given algorithm to a given level: Consider how much extra performance is given vs. the time it takes to run.
	If the algorithm takes ages to run and doesn't make the code much faster, add it to O3.
	If it's near instant and makes the code much much faster, add it to O1.
	O0 always disables every optimization, even the obvious ones. This ensures clarity between the compiler and programmer. As in, the programmer can tell what the compiler is doing, based on the changes to code in O0 vs. others.
	*/
	enum class OptimizeLevel
	{
		//equivalent to O0; debug (none) level
		NONE,
		//equivalent to O1; just do low-hanging fruit
		BASIC,
		//equivalent to O2; does more optimizations
		BALANCED,
		//equivalent to O3; does EVERY optimization
		PERFORMANCE
	};

	class Compiler
	{
	private:
		OptimizeLevel optimizeLvl = OptimizeLevel::BALANCED;
		std::map<std::string, std::string> dynTypes;
		std::vector<Statement*> ast;
	public:
		Compiler() = default;
		virtual ~Compiler();

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
		  and ideally be a built-in type.
		*/
		Compiler* setDynamicType(std::string inner, std::string concrete);

		/*
		Uses existing CBIR code to create an internal AST which can then be compiled.

		CBIR is merely the binary representation of a Caliburn source file, and is
		the format recommended for redistribution.
		*/
		void parseCBIR(std::vector<uint32_t>* cbir);

		/*
		Parses source code text into an internal AST.

		This method does NOT imply endorsement of the redistribution of shader
		source code. Please package your shaders as CBIR for redistribution.
		*/
		void parseText(std::string text);

		/*
		Compiles the parsed code into a final set of shaders. Each shader is in
		SPIR-V format, and contains all the metadata needed to interface with Vulkan.

		shaderName: The name of the shader object to compile.
		shaderDest: The final vector for the final Vulkan shaders.

		Returns a boolean; True for a successful compilation, and false for
		critical errors.

		An empty shaderName will result in a failed compilation.
		*/
		bool compileShaders(std::string shaderName, std::vector<Shader>& shaderDest);

	};

}
