
#pragma once

#include <functional>
#include <vector>

/*
C++ Integrated Query is a set of functions meant to integrate vector-centric functionality into C++
*/
namespace cinq
{
	template<typename In, typename Out>
	std::vector<Out> map(std::vector<In> in, std::function<Out(const In&)> fn)
	{
		auto out = std::vector<Out>(in.size());

		for (auto const& i : in)
		{
			out.push_back(fn(i));

		}

		return out;
	}

	template<typename Obj>
	std::vector<Obj> filter(std::vector<Obj> in, std::function<bool(const Obj&)> fn)
	{
		auto out = std::vector<Obj>(in.size());

		for (auto const& i : in)
		{
			if (fn(i))
			{
				out.push_back(i);

			}

		}

		return out;
	}

}
