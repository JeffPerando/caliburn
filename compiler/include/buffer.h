
#pragma once

#include <vector>

#include "basic.h"

namespace caliburn
{
	/*
	Buffer is a read-only container used to maintain an index to a vector.
	*/
	template<typename T>
	struct Buffer
	{
	private:
		const std::vector<T> vec;
		size_t index = 0;
	public:
		Buffer(in<std::vector<T>> backend) : vec(backend) {}

		/*
		Direct accessor
		*/
		const T& operator[](size_t i) const
		{
			return vec.at(i);
		}

		/*
		Returns true if at least count elements are available within the vector from the current offset
		*/
		bool hasRem(size_t count) const
		{
			return remaining() >= count;
		}

		/*
		Returns true if the current offset points to a valid element in the vector
		*/
		bool hasCur() const
		{
			return index < vec.size();
		}

		/*
		Fetches the element in vector at offset
		*/
		const T& cur() const
		{
			return vec.at(index);
		}

		/*
		Increments offset, then returns the element at the new offset.
		*/
		const T& next()
		{
			++index;
			return vec.at(index);
		}

		/*
		Saves element at current index, increments index, then returns.
		*/
		const T& take()
		{
			auto const& x = vec.at(index);
			++index;
			return x;
		}

		/*
		Returns the element at (index + offset).
		*/
		const T& peek(size_t off) const
		{
			return vec.at(index + off);
		}

		/*
		Returns the element at (index - offset).
		*/
		const T& peekBack(size_t off)
		{
			return vec.at(index - off);
		}

		const T& first()
		{
			return vec.front();
		}

		const T& last()
		{
			return vec.back();
		}

		/*
		Increments the current offset. That's it.
		*/
		void consume(size_t count = 1)
		{
			index += count;
		}

		/*
		Returns current index/offset
		*/
		size_t offset() const
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

		/*
		Manually sets the offset.

		Despite the name, no check is done to ensure the passed offset is less than the current one.

		At this rate, should offset be a public member? *shrug*
		*/
		void revertTo(size_t i)
		{
			index = i;
		}

		/*
		Subtracts the current offset by the passed offset.

		If this will result in an integer underflow, nothing happens.
		*/
		void rewind(size_t off = 1)
		{
			if (off <= index)
			{
				index -= off;
			}
		}

	};

}