
#pragma once

#ifndef __cplusplus
#error Caliburn requires C++

#elif __cplusplus < 201703L
#error Caliburn requires C++17

#ifdef _MSC_VER
#error Try adding /Zc:__cplusplus to your compiler options
#endif

#else

#include <map>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

#if defined(_WIN32)

#ifdef CBRN_NO_IMPORT
#define CBRN_API
#elif defined(CBRN_BUILD_DLL)
#define CBRN_API __declspec(dllexport)
#else
#define CBRN_API __declspec(dllimport)
#endif

#else
#define CBRN_API
#endif

namespace caliburn
{
	/*
	Sets the type of compilation target

	NOTE: Currently unused; Caliburn currently only supports GPU compilation
	*/
	enum class HostTarget : uint32_t
	{
		CPU, GPU
	};

	/*
	Sets the host language

	NOTE: Currently unused; Caliburn currently only supports SPIR-V
	*/
	enum class GPUTarget : uint32_t
	{
		SPIRV, DXIL, METAL
	};

	/*
	Enum denoting how much to optimize code.
	*/
	enum class OptimizeLevel : uint32_t
	{
		DEBUG, //O0
		BASIC, //O1
		BALANCED, //O2
		PERFORMANCE //O3
	};

	/*
	Enum denoting how much to validate generated CLLR

	Full should only be used for development purposes
	*/
	enum class ValidationLevel : uint32_t
	{
		//Disables validation entirely
		NONE,
		//Only checks for things a programmer could mess up on;
		BASIC,
		//Checks everything
		FULL,
		//Checks everything and bugs you if debug symbols are missing
		DEV
	};

	struct CompilerSettings
	{
		GPUTarget gpuTarget = GPUTarget::SPIRV;
		OptimizeLevel o = OptimizeLevel::DEBUG;

		/*
		Sets the degree of validation for the compiler.

		NONE disables validation entirely.
		BASIC catches routine errors that can be created by a programmer.
		FULL catches any and all problems in the output code.
		*/
		ValidationLevel vLvl = ValidationLevel::BASIC;

		/*
		In Caliburn, a type can be defined as "dynamic", which means the actual
		type is determined externally. This is where they're defined. This allows
		devs to write one shader that can use both, say, FP16 and FP32.

		- inner must correlate to a dynamic type within a shader being compiled.
		- concrete must correlate to an existing type within a compiled shader,
		  and ideally be a built-in type.
		*/
		std::map<std::string, std::string> dynTypes;
		uint32_t errorContextLines = 3;

		char _padding[980]{};

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

	struct Shader
	{
		const ShaderType type;
		const std::vector<uint32_t> code;

		std::vector<VertexInputAttribute> inputs;
		std::vector<DescriptorSet> sets;

		Shader(ShaderType t, const std::vector<uint32_t>& c) : type(t), code(c) {}

	};

	struct ShaderResult
	{
		std::vector<std::unique_ptr<Shader>> shaders;
		std::vector<std::string> errors;

		bool success() const
		{
			return errors.empty();
		}

	};

	struct Compiler
	{
	private:
		std::shared_ptr<CompilerSettings> settings;
		
	public:
		CBRN_API Compiler() : settings(std::make_shared<CompilerSettings>()) {}
		CBRN_API Compiler(const CompilerSettings& cs) : settings(std::make_shared<CompilerSettings>(cs)) {}
		CBRN_API virtual ~Compiler() = default;

		/*
		Compiles raw source code into a set of shaders.

		src: The source code, in ASCII. UTF-8 is currently not supported.
		shaderName: The name of the shader object to compile. Cannot be empty.
		
		Returns a ShaderResult, which contains a vector of shaders and error messages.
		*/
		CBRN_API ShaderResult compileSrcShaders(const std::string& src, const std::string& shaderName);

	};

}

#endif
