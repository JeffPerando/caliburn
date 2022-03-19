#include "spirvtrans.h"

using namespace caliburn;

bool CodeTransform::compare(Transformer* trans, std::vector<uint32_t> codes, uint64_t offset)
{
	if (codes.size() - offset < points.size())
	{
		return false;
	}

	uint64_t pointIndex = 0;

	for (uint64_t i = offset; i < codes.size(); ++i)
	{
		if (pointIndex == points.size())
		{
			return true;
		}

		CodePoint* point = points.at(pointIndex);
		auto code = codes.at(i + offset);

		if (!point->compare(code))
		{
			return false;
		}

		++pointIndex;

		if (point->type == CodeCompareType::CACHE)
		{
			trans->cache(point->cacheID, code);

		}
		else if (point->type == CodeCompareType::INSTRUCTION_CACHE_LEN)
		{
			trans->cache(point->cacheID, SpvOp(code).words());

		}

	}

}
