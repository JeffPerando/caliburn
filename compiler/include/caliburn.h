
#pragma once

#ifdef __cplusplus

#if __cplusplus < 201703L
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

#if defined(CBRN_NO_IMPORT)
#define CBRN_API
#elif defined(CBRN_BUILD_DLL)
#define CBRN_API __declspec(dllexport)
#else
#define CBRN_API __declspec(dllimport)
#endif

#else
#define CBRN_API
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace caliburn
{
	/*
	Sets the type of compilation target

	NOTE: Currently unused; Caliburn currently only supports GPU compilation
	*/
	enum class CBRN_API HostTarget : uint32_t
	{
		CPU, GPU
	};

	/*
	Sets the host language

	NOTE: Currently unused; Caliburn currently only supports SPIR-V
	*/
	enum class CBRN_API GPUTarget : uint32_t
	{
		SPIRV, DXIL, METAL
	};

	/*
	Enum denoting how much to optimize code.
	*/
	enum class CBRN_API OptimizeLevel : uint32_t
	{
		DEBUG, //O0
		BASIC, //O1
		BALANCED, //O2
		PERFORMANCE //O3
	};

	//FIXME not used atm; awaiting writing the full validation layer
	enum class CBRN_API ValidationLevel : uint32_t
	{
		//Disables validation entirely
		NONE,
		//Only checks for things a programmer could mess up on;
		BASIC,
		//Checks everything
		FULL
	};

	struct CBRN_API CompilerSettings
	{
		OptimizeLevel o = OptimizeLevel::DEBUG;
		ValidationLevel vLvl = ValidationLevel::BASIC;
		std::map<std::string, std::string> dynTypes;

	};

	struct CBRN_API DescriptorSet
	{
		std::string name;
		uint32_t binding;
		uint32_t type;

	};

	struct CBRN_API VertexInputAttribute
	{
		std::string name;
		uint32_t location;
		uint32_t format;

	};

	enum class CBRN_API ShaderType
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

	struct CBRN_API Shader
	{
		const ShaderType type;
		const std::unique_ptr<std::vector<uint32_t>> code;

		std::vector<VertexInputAttribute> inputs;
		std::vector<DescriptorSet> sets;

		Shader(ShaderType t, std::unique_ptr<std::vector<uint32_t>>& c) : type(t), code(std::move(c)) {}

	};

	struct CBRN_API Compiler
	{
	private:
		std::shared_ptr<CompilerSettings> settings;
		std::vector<std::string> allErrors;
		
	public:
		Compiler() : settings(std::make_shared<CompilerSettings>()) {}
		Compiler(const CompilerSettings& cs) : settings(std::make_shared<CompilerSettings>(cs)) {}
		Compiler(std::shared_ptr<CompilerSettings> cs) : settings(cs) {}
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
		target: The GPU IR to compile to. Currently only SPIR-V is supported

		Returns the set of compiled shaders
		*/
		std::vector<std::unique_ptr<Shader>> compileSrcShaders(std::string src, std::string shaderName, GPUTarget target = GPUTarget::SPIRV);

		std::vector<std::string> getErrors() const;
		
	};

}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
#endif
