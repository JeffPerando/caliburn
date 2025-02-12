
/*
Contains necessary constants, and data types necessary for working with Caliburn.
*/

#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "basic.h"

#define CBRN_NO_IMPORT
#include "caliburn.h"

namespace caliburn
{
	auto static constexpr MIN_INT_BITS = 8;
	auto static constexpr MAX_INT_BITS = 64;
	
	auto static constexpr MIN_FLOAT_BITS = 16;
	auto static constexpr MAX_FLOAT_BITS = 64;

	auto static constexpr MIN_VECTOR_LEN = 2;
	auto static constexpr MAX_VECTOR_LEN = 4;

	enum class ShaderIOVarType
	{
		INPUT,
		OUTPUT
	};

	struct IOVar
	{
		std::string_view name;
		ShaderIOVarType type;
		uint32_t index;
	};

	//TODO audit these texture variants
	enum class TextureKind
	{
		_1D, _2D, _3D, CUBEMAP//, _1D_BUF
	};

	static const HashMap<TextureKind, std::string_view> TEX_TYPES = {
		{ TextureKind::_1D, "1D" },
		{ TextureKind::_2D, "2D" },
		{ TextureKind::_3D, "3D" },
		{ TextureKind::CUBEMAP, "Cube" },
	};

	static const HashMap<TextureKind, std::string_view> TEX_SAMPLE_VECS = {
		{ TextureKind::_1D, "fp32" },
		{ TextureKind::_2D, "vec2" },
		{ TextureKind::_3D, "vec3" },
		{ TextureKind::CUBEMAP, "vec3" },
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

	static const HashMap<ShaderType, std::string_view> SHADER_TYPE_NAMES = {
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

	struct ExprModifiers
	{
		uint32_t
			PUBLIC : 1,
			PRIVATE : 1,
			SHARED : 1,
			STATIC : 1;

		operator uint32_t() const
		{
			return *(uint32_t*)this;
		}

	};

}
