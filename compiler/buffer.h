
#pragma once

#include <vector>

namespace caliburn
{
	//A buffer in this case is a read-only container used to maintain a current index to a vector
	//might implement the ability to add, but imo that wouldn't be too useful
	template<typename T>
	class buffer
	{
	private:
		std::vector<T>* const vec;
		size_t index = 0;
	public:
		buffer() : buffer(nullptr) {}
		buffer(std::vector<T>* backend) : vec(backend) {}

		bool hasNext()
		{
			return vec->size() > index;
		}

		//fetch current
		T current()
		{
			return vec->at(index);
		}

		//increment and fetch
		T next()
		{
			index += 1;
			return vec->at(index);
		}

		//fetch upcoming
		T peek(size_t offset = 1)
		{
			return vec->at(index + offset);
		}

		//increment
		void consume(size_t count = 1)
		{
			index += count;

		}

		size_t currentIndex()
		{
			return index;
		}

		void revertTo(size_t i)
		{
			index = i;
		}

	};

}