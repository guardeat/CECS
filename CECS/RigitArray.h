#ifndef RIGITARRAY_H
#define RIGITARRAY_H

#include <algorithm>
#include <memory>
#include <vector>
#include <type_traits>

#include "RigitAllocator.h"

namespace CECS
{
	template<typename T>
	class RigitIterator
	{
	private:
		using MultiArray = T**;

		MultiArray arrays;
		size_t arraySize;
		size_t itemIndex;

	public:
		RigitIterator(MultiArray arrays, size_t arraySize, size_t itemIndex)
			:arrays{ arrays }, arraySize{ arraySize }, itemIndex{ itemIndex }
		{
		}

		T& operator*()
		{
			return arrays[itemIndex / arraySize][itemIndex % arraySize];
		}

		const T& operator*() const
		{
			return arrays[itemIndex / arraySize][itemIndex % arraySize];
		}

		RigitIterator& operator++()
		{
			++itemIndex;
			return *this;
		}

		bool operator==(const RigitIterator& right) const
		{
			return itemIndex == right.itemIndex;
		}

		bool operator!=(const RigitIterator& right) const
		{
			return itemIndex != right.itemIndex;
		}
	};

	template<typename T, typename Allocator = RigitAllocator<T>>
	class RigitArray
	{
	private:
		using Array = T*;
		using ArrayContainer = std::vector<Array>;

		ArrayContainer arrays;
		Allocator allocator;
		size_t arraySize;
		size_t itemCount{};

	public:
		RigitArray() : arraySize{ allocator.createSize() }
		{
		}

		RigitArray(const RigitArray& rigitArray) : RigitArray(std::move(rigitArray.copy()))
		{
		}

		RigitArray(RigitArray&& rigitArray) noexcept :
			arrays{ std::move(rigitArray.arrays) },
			allocator{ std::move(rigitArray.allocator) },
			arraySize{ rigitArray.arraySize },
			itemCount{ rigitArray.itemCount }
		{
			rigitArray.itemCount = 0;
		}

		RigitArray& operator=(const RigitArray& rigitArray)
		{
			clear();
			*this = std::move(rigitArray.copy());
			return *this;
		}

		RigitArray& operator=(RigitArray&& rigitArray) noexcept
		{
			clear();
			arrays = std::move(rigitArray.arrays);
			allocator = std::move(rigitArray.allocator);
			arraySize = rigitArray.arraySize;
			itemCount = rigitArray.itemCount;
			rigitArray.itemCount = 0;
			return *this;
		}

		virtual ~RigitArray()
		{
			clear();
		}

		void pushBack(const T& item)
		{
			pushBack(T{ item });
		}

		void pushBack(T&& item)
		{
			increaseCapacity(itemCount + 1);
			constuct(itemCount, std::move(item));
			++itemCount;
		}

		void popBack()
		{
			destroy(itemCount - 1);
			--itemCount;
			decreaseCapacity(itemCount);
		}

		T& get(size_t index)
		{
			return arrays[index / arraySize][index % arraySize];
		}

		const T& get(size_t index) const
		{
			return arrays[index / arraySize][index % arraySize];
		}

		void set(size_t index, const T& item)
		{
			get(index) = item;
		}

		void set(size_t index, T&& item)
		{
			get(index) = std::move(item);
		}

		T& operator[](size_t index)
		{
			return get(index);
		}

		const T& operator[](size_t index) const
		{
			return get(index);
		}

		T& back()
		{
			return get(itemCount - 1);
		}

		const T& back() const
		{
			return get(itemCount - 1);
		}

		void swap(size_t left, size_t right)
		{
			std::swap(get(left), get(right));
		}

		void clear()
		{
			for (size_t index{}; index < itemCount; ++index)
			{
				destroy(index);
			}
			decreaseCapacity(0);
			itemCount = 0;
		}

		size_t size() const
		{
			return itemCount;
		}

		size_t capacity() const
		{
			return arraySize * arrays.size();
		}

		bool empty() const
		{
			return !static_cast<bool>(itemCount);
		}

		RigitIterator<T> begin()
		{
			return RigitIterator<T>{arrays.data(), arraySize, 0};
		}

		RigitIterator<T> end()
		{
			return RigitIterator<T>{nullptr, 0, itemCount};
		}

		RigitArray copy() const
		{
			RigitArray<T, Allocator> out;
			for (size_t index{}; index < itemCount; ++index)
			{
				out.pushBack(get(index));
			}
			return out;
		}

	private:
		void constuct(size_t index, T&& item)
		{
			std::allocator_traits<Allocator>::construct(allocator, &get(index), std::move(item));
		}

		void destroy(size_t index)
		{
			std::allocator_traits<Allocator>::destroy(allocator, &get(index));
		}

		void increaseCapacity(size_t newCapacity)
		{
			while (newCapacity > capacity())
			{
				arrays.push_back(std::allocator_traits<Allocator>::allocate(allocator, arraySize));
			}
		}

		void decreaseCapacity(size_t newCapacity)
		{
			if (newCapacity)
			{
				newCapacity += arraySize;
			}

			while (capacity() > newCapacity)
			{
				Array removal{ arrays.back() };
				std::allocator_traits<Allocator>::deallocate(allocator, removal, arraySize);
				arrays.pop_back();
			}
		}
	};
}

#endif