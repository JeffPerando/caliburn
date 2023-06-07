
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "basic.h"

/*
Contains necessary constants, data types, and functions necessary for parsing, interpreting,
and compiling Caliburn code.
*/
namespace caliburn
{
	auto static constexpr MIN_INT_BITS = 8;
	auto static constexpr MAX_INT_BITS = 512;
	
	auto static constexpr MIN_FLOAT_BITS = 16;
	auto static constexpr MAX_FLOAT_BITS = 128;

	auto static constexpr MIN_VECTOR_LEN = 2;
	auto static constexpr MAX_VECTOR_LEN = 4;

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

	static const std::map<std::string, ShaderType> SHADER_TYPES = {
		/*
		{"main",		ShaderType::COMPUTE},
		*/
		{"vertex",		ShaderType::VERTEX},
		{"frag",		ShaderType::FRAGMENT},
		/*
		{"tessCtrl",	ShaderType::TESS_CTRL},
		{"tessEval",	ShaderType::TESS_EVAL},
		*/
		{"geom", 		ShaderType::GEOMETRY},
		/*
		{"rayGen",   	ShaderType::RT_GEN},
		{"rayClose",	ShaderType::RT_CLOSE},
		{"rayHit",		ShaderType::RT_ANY_HIT},
		{"rayInt",		ShaderType::RT_INTERSECT},
		{"rayMiss",		ShaderType::RT_MISS},
		{"task",		ShaderType::TASK},
		{"mesh",		ShaderType::MESH}
		*/
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
		ShaderType type;
		std::vector<uint32_t> spirv;
		std::vector<VertexInputAttribute> inputs;
		std::vector<DescriptorSet> sets;

	};

	struct StmtModifiers
	{
		uint32_t
			PUBLIC : 1,
			PROTECTED : 1,
			PRIVATE : 1,
			SHARED : 1,
			STATIC : 1;

		StmtModifiers()
		{
			PUBLIC = 0;
			PROTECTED = 0;
			PRIVATE = 0;
			SHARED = 0;
			STATIC = 0;
		};

	};

}
