
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
		size_t offset = 0;
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
			return remaining() > 0;
		}

		/*
		Returns true if the current offset points to a valid element in the vector
		*/
		bool hasCur() const
		{
			return offset < vec.size();
		}

		/*
		Fetches the element in vector at offset
		*/
		const T& cur() const
		{
			return vec.at(offset);
		}

		/*
		Incrments offset, then returns the element at it.
		*/
		const T& next()
		{
			++offset;
			return vec.at(offset);
		}

		/*
		Returns the element at (offset + offset).
		*/
		const T& peek(size_t off) const
		{
			return vec.at(offset + off);
		}

		/*
		Returns the element at (offset - offset).
		*/
		const T& peekBack(size_t off)
		{
			return vec.at(offset - off);
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
			offset += count;
		}

		/*
		Returns current index/offset
		*/
		size_t index() const
		{
			return offset;
		}

		size_t length() const
		{
			return vec.size();
		}

		size_t remaining() const
		{
			return length() - offset;
		}

		/*
		Manually sets the offset.

		Despite the name, no check is done to ensure the passed offset is less than the current one.

		At this rate, should offset be a public member? *shrug*
		*/
		void revertTo(size_t i)
		{
			offset = i;
		}

		/*
		Subtracts the current offset by the passed offset.

		If this will result in an integer underflow, nothing happens.
		*/
		void rewind(size_t off = 1)
		{
			if (off <= offset)
			{
				offset -= off;
			}
		}

	};

}