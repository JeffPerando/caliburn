
#pragma once

#include <vector>

#include "basic.h"

namespace caliburn
{
	//A buffer in this case is a read-only container used to maintain an index to a vector.
	template<typename T>
	struct Buffer
	{
	private:
		const std::vector<T> vec;
		size_t index = 0;
	public:
		Buffer(ref<std::vector<T>> backend) : vec(backend) {}

		const T& operator[](size_t i) const
		{
			return vec.at(i);
		}

		bool hasNext(size_t count = 1) const
		{
			return remaining() >= count;
		}

		bool hasRem() const
		{
			return index != vec.size();
		}

		//fetch current
		const T& current() const
		{
			return vec.at(index);
		}

		//increment and fetch
		const T& next()
		{
			++index;
			return vec.at(index);
		}

		//decrement and fetch
		const T& prev(size_t off = 1)
		{
			index -= off;
			return vec.at(index);
		}

		//fetch upcoming
		const T& peek(size_t off = 1) const
		{
			return vec.at(index + off);
		}

		//fetch previous
		const T& peekBack(size_t off = 1)
		{
			return vec.at(index - off);
		}

		//increment
		void consume(size_t count = 1)
		{
			index += count;

		}

		size_t currentIndex() const
		{
			return index;
		}

		size_t length() const
		{
			return vec.size();
		}

		size_t remaining() const
		{
			return length() - index;
		}

		bool inRange(size_t offset) const
		{
			return (index + offset) < length();
		}

		void revertTo(size_t i)
		{
			index = i;
		}

		void rewind(size_t i = 1)
		{
			if (i > index)
				return;
			index -= i;
		}

	};

}