
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "langcore.h"

namespace caliburn
{
	struct Statement;

	class Compiler
	{
	private:
		CompilerSettings settings;
		HashMap<std::string, std::string> dynTypes;
		std::vector<uptr<Statement>> ast;
	public:
		Compiler() = default;
		virtual ~Compiler() {}

		/*
		Sets the optimization level for the compiler. See OptimizeLevel enum.
		*/
		void o(OptimizeLevel lvl);

		/*
		Enables CLLR validation. Will lower compilation performance.
		*/
		void enableValidation();

		/*
		In Caliburn, a type can be defined as "dynamic", which means the actual
		type is determined externally. This is where they're defined. This allows
		devs to write one shader that can use both, say, FP16 and FP32.

		- inner must correlate to a dynamic type within a shader being compiled.
		- concrete must correlate to an existing type within a compiled shader,
		  and ideally be a built-in type.
		*/
		void setDynamicType(std::string inner, std::string concrete);

		/*
		Uses existing CBIR code to create an internal AST which can then be compiled.

		CBIR is merely the binary representation of a Caliburn source file, and is
		the format recommended for redistribution.

		Returns whether the CBIR produced an AST or not
		*/
		bool parseCBIR(ref<std::vector<uint32_t>> cbir);

		/*
		Parses source code text into an internal AST.

		This method does NOT imply endorsement of the redistribution of shader
		source code. Please package your shaders as CBIR for redistribution.

		Returns whether the input string produced an AST or not
		*/
		bool parseText(std::string text);

		/*
		Compiles the parsed code into a final set of shaders. Each shader is in
		SPIR-V format, and contains all the metadata needed to interface with Vulkan.

		shaderName: The name of the shader object to compile.
		shaderDest: The final vector for the final Vulkan shaders.

		Returns a boolean; True for a successful compilation, and false for
		critical errors.

		An empty shaderName will result in a failed compilation.
		*/
		bool compileShaders(std::string shaderName, ref<std::vector<uptr<Shader>>> shaderDest);

	};

}
