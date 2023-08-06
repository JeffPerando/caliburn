
#pragma once

#if __cplusplus < 201703L
#error Caliburn requires C++17; also MSVC doesn't set __cplusplus correctly, so check that if you're using it
#else

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "langcore.h"

namespace caliburn
{
	struct Compiler
	{
	private:
		sptr<CompilerSettings> settings;
		std::vector<std::string> allErrors;
		
	public:
		Compiler() : settings(new_sptr<CompilerSettings>()) {}
		Compiler(ref<CompilerSettings> cs) : settings(new_sptr<CompilerSettings>(cs)) {}
		virtual ~Compiler() {}

		/*
		Sets the optimization level for the compiler. See OptimizeLevel enum.
		*/
		void o(OptimizeLevel lvl);

		/*
		Sets the degree of validation for the compiler.

		NONE disables validation entirely.
		BASIC catches routine errors that can be created by a programmer.
		FULL catches any and all problems in the output code.
		*/
		void setValidationLvl(ValidationLevel lvl);

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
		Compiles raw source code into a set of shaders.

		src: The source code, in ASCII. UTF-8 is currently not supported.
		shaderName: The name of the shader object to compile. Cannot be empty.

		Returns the set of shaders
		*/
		std::vector<uptr<Shader>> compileSrcShaders(std::string src, std::string shaderName);

		std::vector<std::string> getErrors() const;
		
	};

}

#endif
