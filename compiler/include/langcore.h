
/*
Contains necessary constants, data types, and functions necessary for working with Caliburn.
*/

#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "basic.h"

namespace caliburn
{
	enum class Target : uint32_t
	{
		CPU, GPU
	};

	//TODO reconsider
	enum class TargetOutput : uint32_t
	{
		SPIRV, LLVM, DXIL
	};

	/*
	Enum denoting how much to optimize code.

	Regarding whether to add a given algorithm to a given level: Consider how much extra performance is given vs. the time it takes to run.
	If the algorithm takes ages to run and doesn't make the code much faster, add it to O3.
	If it's near instant and makes the code much much faster, add it to O1.
	DEBUG disables every optimization, even the obvious ones.
	*/
	enum class OptimizeLevel
	{
		//equivalent to O0. just show me the raw output
		DEBUG,
		//equivalent to O1; just do low-hanging fruit
		BASIC,
		//equivalent to O2; does more optimizations
		BALANCED,
		//equivalent to O3; does EVERY optimization
		PERFORMANCE
	};

	//FIXME not used atm; awaiting writing the full validation layer
	enum class ValidationLevel : uint32_t
	{
		//Disables validation entirely
		NONE,
		//Only checks for things a programmer could mess up on;
		BASIC,
		//Checks everything
		FULL
	};

	struct CompilerSettings
	{
		OptimizeLevel o = OptimizeLevel::DEBUG;
		ValidationLevel vLvl = ValidationLevel::BASIC;
		std::map<std::string, std::string> dynTypes;

	};

	auto static constexpr MIN_INT_BITS = 8;
	auto static constexpr MAX_INT_BITS = 512;
	
	auto static constexpr MIN_FLOAT_BITS = 16;
	auto static constexpr MAX_FLOAT_BITS = 128;

	auto static constexpr MIN_VECTOR_LEN = 2;
	auto static constexpr MAX_VECTOR_LEN = 4;

	enum class TypeCategory : uint32_t
	{
		VOID,
		FLOAT,
		INT,
		VECTOR,
		MATRIX,
		ARRAY,
		STRUCT,
		BOOLEAN,
		POINTER
		//TUPLE
		//STRING

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

	static const HashMap<std::string_view, ShaderType> SHADER_TYPES = {
		//{"main",		ShaderType::COMPUTE},
		{"vertex",		ShaderType::VERTEX},
		{"frag",		ShaderType::FRAGMENT},
		//{"tessCtrl",	ShaderType::TESS_CTRL},
		//{"tessEval",	ShaderType::TESS_EVAL},
		{"geom", 		ShaderType::GEOMETRY},
		//{"rayGen",   	ShaderType::RT_GEN},
		//{"rayClose",	ShaderType::RT_CLOSE},
		//{"rayHit",		ShaderType::RT_ANY_HIT},
		//{"rayInt",		ShaderType::RT_INTERSECT},
		//{"rayMiss",		ShaderType::RT_MISS},
		//{"task",		ShaderType::TASK},
		//{"mesh",		ShaderType::MESH}
	};

	static const HashMap<ShaderType, std::string> SHADER_TYPE_NAMES = {
		//{ShaderType::COMPUTE, "main"},
		{ShaderType::VERTEX,		"vertex"},
		{ShaderType::FRAGMENT,		"frag"},
		//{ShaderType::TESS_CTRL,	"tessCtrl"},
		//{ShaderType::TESS_EVAL,	"tessEval"},
		{ShaderType::GEOMETRY,		"geom"},
		//{ShaderType::RT_GEN,		"rayGen"},
		//{ShaderType::RT_CLOSE,	"rayClose"},
		//{ShaderType::RT_ANY_HIT,	"rayHit"},
		//{ShaderType::RT_INTERSECT	"rayInt"},
		//{ShaderType::RT_MISS,		"rayMiss"},
		//{ShaderType::TASK,		"task"},
		//{ShaderType::MESH,		"mesh"}
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
		const ShaderType type;
		const uptr<std::vector<uint32_t>> spirv;

		std::vector<VertexInputAttribute> inputs;
		std::vector<DescriptorSet> sets;

		Shader(ShaderType t, ref<uptr<std::vector<uint32_t>>> spv) : type(t), spirv(std::move(spv)) {}

	};

	struct StmtModifiers
	{
		uint32_t
			PUBLIC : 1,
			PROTECTED : 1,
			PRIVATE : 1,
			SHARED : 1,
			STATIC : 1;

		operator uint32_t() const
		{
			return *(uint32_t*)this;
		}

	};

}
