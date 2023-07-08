
#pragma once

#include <vector>

#include "basic.h"

namespace caliburn
{
	//A buffer in this case is a read-only container used to maintain an index to a vector.
	template<typename T>
	class buffer
	{
	private:
		const ptr<std::vector<T>> vec;
		size_t index = 0;
	public:
		buffer(ptr<std::vector<T>> backend) : vec(backend) {}

		bool hasNext() const
		{
			return (index + 1) < vec->size();
		}

		//fetch current
		T current() const
		{
			return vec->at(index);
		}

		//increment and fetch
		T next()
		{
			++index;
			return vec->at(index);
		}

		T prev(size_t off = 1)
		{
			return vec->at(index - off);
		}

		//fetch upcoming
		T peek(size_t offset = 1) const
		{
			return vec->at(index + offset);
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
			return vec->size();
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