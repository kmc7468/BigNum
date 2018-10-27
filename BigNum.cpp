/* MIT License
 *
 * Copyright (c) 2018 kmc7468, kiwiyou
 *
 * Permission is hereby granted, reset of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/////////////////////////////////////////////////////////////////
///// Includes
/////////////////////////////////////////////////////////////////

#include "BigNum.hpp"

#include <algorithm>
#include <cstdlib>
#include <new>

/////////////////////////////////////////////////////////////////
///// Definitions
/////////////////////////////////////////////////////////////////

#ifdef _BIGNUM_HAS_NAMESPACE
namespace _BIGNUM_HAS_NAMESPACE
{
#endif

bigint::bigint(std::int32_t integer)
	: capacity_(2)
{
	data_ = reinterpret_cast<block_type*>(std::calloc(2, sizeof(block_type)));

	if (!data_)
	{
		capacity_ = 0;
		throw std::bad_alloc();
	}

	data_[0] = static_cast<block_type>(integer);

	if (integer < 0)
	{
		sign_ = true;
		if ((data_[0] = ~data_[0] + 1) == 0) // Overflow
		{
			data_[1] = 1;
		}
	}
}
bigint::bigint(std::uint32_t integer)
	: capacity_(1)
{
	data_ = reinterpret_cast<block_type*>(std::calloc(1, sizeof(block_type)));

	if (!data_)
	{
		capacity_ = 0;
		throw std::bad_alloc();
	}

	*data_ = integer;
}
bigint::bigint(std::int64_t integer)
	: capacity_(3)
{
	data_ = reinterpret_cast<block_type*>(std::calloc(3, sizeof(block_type)));

	if (!data_)
	{
		capacity_ = 0;
		throw std::bad_alloc();
	}

	data_[0] = static_cast<block_type>(static_cast<std::uint64_t>(integer) & 0xFFFFFFFF);
	data_[1] = static_cast<block_type>(static_cast<std::uint64_t>(integer) >> 32);

	if (integer < 0)
	{
		sign_ = true;
		data_[1] = ~data_[1];
		if ((data_[0] = ~data_[0] + 1) == 0) // Overflow
		{
			if ((data_[1] += 1) == 0) // Overflow
			{
				data_[2] = 1;
			}
		}
	}
}
bigint::bigint(std::uint64_t integer)
	: capacity_(2)
{
	data_ = reinterpret_cast<block_type*>(std::calloc(2, sizeof(block_type)));

	if (!data_)
	{
		capacity_ = 0;
		throw std::bad_alloc();
	}

	data_[0] = static_cast<block_type>(integer & 0xFFFFFFFF);
	data_[1] = static_cast<block_type>(integer >> 32);
}
bigint::bigint(const bigint& integer)
	: capacity_(integer.capacity_), sign_(integer.sign_)
{
	data_ = reinterpret_cast<block_type*>(std::calloc(capacity_, sizeof(block_type)));

	if (!data_)
	{
		capacity_ = 0;
		throw std::bad_alloc();
	}

	std::copy(integer.data_, integer.data_ + integer.capacity_, data_);
}
bigint::bigint(bigint&& integer) noexcept
	: data_(integer.data_), capacity_(integer.capacity_), sign_(integer.sign_)
{
	integer.data_ = nullptr;
	integer.capacity_ = 0;
}
bigint::~bigint()
{
	reset();
}

bigint& bigint::operator=(const bigint& integer)
{
	if (capacity_ < integer.capacity_)
	{
		block_type* const old_data = data_;
		const std::size_t old_capacity = capacity_;

		data_ = reinterpret_cast<block_type*>(std::realloc(data_, sizeof(block_type) * integer.capacity_));
		capacity_ = integer.capacity_;

		if (!data_)
		{
			std::free(old_data);
			capacity_ = 0;
			throw std::bad_alloc();
		}

		std::fill(data_ + old_capacity, data_ + capacity_, 0);
	}
		
	sign_ = integer.sign_;
	std::copy(integer.data_, integer.data_ + integer.capacity_, data_);

	if (capacity_ > integer.capacity_)
	{
		std::fill(data_ + integer.capacity_, data_ + capacity_, 0);
	}

	return *this;
}
bigint& bigint::operator=(bigint&& integer) noexcept
{
	data_ = integer.data_;
	capacity_ = integer.capacity_;
	sign_ = integer.sign_;

	integer.data_ = nullptr;
	integer.capacity_ = 0;

	return *this;
}

void bigint::reset() noexcept
{
	std::free(data_);

	data_ = nullptr;
	capacity_ = 0;
	sign_ = false;
}
void bigint::swap(bigint& integer) noexcept
{
	if (this == &integer) return;

	std::swap(data_, integer.data_);
	std::swap(capacity_, integer.capacity_);
	std::swap(sign_, integer.sign_);
}

void bigint::reserve(std::size_t new_capacity)
{
	if (new_capacity > capacity_)
	{
		block_type* const old_data = data_;
		const std::size_t old_capacity = capacity_;

		data_ = reinterpret_cast<block_type*>(std::realloc(data_, sizeof(block_type) * new_capacity));
		capacity_ = new_capacity;

		if (!data_)
		{
			std::free(old_data);
			capacity_ = 0;
			throw std::bad_alloc();
		}

		std::fill(data_ + old_capacity, data_ + capacity_, 0);
	}
}
void bigint::shrink_to_fit()
{
	if (data_)
	{
		block_type* const old_data = data_;

		for (std::size_t i = capacity_ - 1; i >= 0; --i)
		{
			if (data_[i] != 0 && i + 1 != capacity_)
			{
				if (!(data_ = reinterpret_cast<block_type*>(std::realloc(data_, capacity_ = i + 1))))
				{
					std::free(old_data);
					capacity_ = 0;
					throw std::bad_alloc();
				}

				return;
			}

			if (i == 0)
			{
				std::free(data_);
				data_ = nullptr;
				capacity_ = 0;

				return;
			}
		}
	}
}

bigint::size_type bigint::capacity() const noexcept
{
	return capacity_;
}

#ifdef _BIGNUM_HAS_NAMESPACE
}
#endif