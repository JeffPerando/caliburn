
#pragma once

#include <initializer_list>
#include <vector>

#include "spirv.h"

namespace caliburn
{
	//TODO shoot myself in the foot for this one
	using namespace spirv;

	struct CodeTransform;
	struct CodeResult;
	struct Transformer;

	enum class CodeCompareType
	{
		DIRECT,
		INSTRUCTION,
		INSTRUCTION_CACHE_LEN,
		CACHE,
		WILDCARD
	};

	enum class CodeResultType
	{
		DIRECT,
		INSTRUCTION,
		INSTRUCTION_CACHED_LEN,
		CACHED
	};

	struct CodePoint
	{
		CodeCompareType type;
		uint32_t value;
		uint32_t cacheID;

		bool compare(uint32_t code)
		{
			if (type == CodeCompareType::DIRECT)
			{
				return code == value;
			}
			else if (type == CodeCompareType::INSTRUCTION
				|| type == CodeCompareType::INSTRUCTION_CACHE_LEN)
			{
				return SpvOp(code).op == value;
			}
			
			return true;
		}

	};

	struct ResultPoint
	{
		CodeResultType type;
		uint32_t value;
	};

	struct CodeTransform
	{
		std::vector<CodePoint*> points;

		CodeTransform() {}
		CodeTransform(std::initializer_list<CodePoint*> ins)
		{
			points.insert(std::end(points), std::begin(ins), std::end(ins));
		}

		bool compare(Transformer* trans, std::vector<uint32_t> codes, uint64_t offset);

	};

	struct CodeResult
	{
		std::vector<ResultPoint> points;

		CodeResult() {}
		CodeResult(std::initializer_list<ResultPoint> ins)
		{
			points.insert(std::end(points), std::begin(ins), std::end(ins));
		}

	};

	struct Transformer
	{
		std::vector<std::pair<CodeTransform*, CodeResult*>> transforms;
		uint32_t dataCache[256];

		void transform(std::vector<uint32_t> code);

		void cache(uint32_t id, uint32_t value)
		{
			if (id < 256)
			{
				dataCache[id] = value;

			}

		}

	};

}
