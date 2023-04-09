
#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "basic.h"
#include "syntax.h"
#include "cllr.h"

/*
Contains necessary constants, data types, and functions necessary for parsing, interpreting,
and compiling Caliburn code.

Also includes the syntax header in case you need that (I believe this does as well)
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
		ptr<const uint32_t> spirv;
		size_t spirv_length;
		std::vector<VertexInputAttribute> inputs;
		std::vector<DescriptorSet> sets;

	};

	struct StorageModifiers
	{
		uint32_t
			PUBLIC : 1,
			PROTECTED : 1,
			PRIVATE : 1,
			SHARED : 1,
			STATIC : 1,
			STRONG : 1;

		StorageModifiers()
		{
			PUBLIC = 0;
			PROTECTED = 0;
			PRIVATE = 0;
			SHARED = 0;
			STATIC = 0;
			STRONG = 0;
		}

	};

}
