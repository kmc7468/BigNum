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

bigint::bigint(const bigint& integer)
	: size_(integer.size_), capacity_(integer.capacity_), sign_(integer.sign_)
{
	data_ = reinterpret_cast<block_type*>(std::malloc(capacity_));

	if (!data_)
	{
		size_ = capacity_ = 0;
		throw std::bad_alloc();
	}

	std::copy(integer.data_, integer.data_ + integer.size_, data_);
}
bigint::bigint(bigint&& integer) noexcept
	: data_(integer.data_), size_(integer.size_), capacity_(integer.capacity_), sign_(integer.sign_)
{
	integer.data_ = nullptr;
	integer.size_ = integer.capacity_ = 0;
}
bigint::~bigint()
{
	reset();
}

bigint& bigint::operator=(const bigint& integer)
{
	block_type* const old_data = data_;

	data_ = reinterpret_cast<block_type*>(std::realloc(data_, integer.capacity_));
	size_ = integer.size_;
	capacity_ = integer.capacity_;
	sign_ = integer.sign_;

	if (!data_)
	{
		std::free(old_data);
		size_ = capacity_ = 0;
		throw std::bad_alloc();
	}

	std::copy(integer.data_, integer.data_ + integer.size_, data_);

	return *this;
}
bigint& bigint::operator=(bigint&& integer) noexcept
{
	data_ = integer.data_;
	size_ = integer.size_;
	capacity_ = integer.capacity_;
	sign_ = integer.sign_;

	integer.data_ = nullptr;
	integer.size_ = integer.capacity_ = 0;

	return *this;
}

void bigint::reset() noexcept
{
	std::free(data_);

	data_ = nullptr;
	size_ = capacity_ = 0;
	sign_ = false;
}
void bigint::swap(bigint& integer) noexcept
{
	if (this == &integer) return;

	std::swap(data_, integer.data_);
	std::swap(size_, integer.size_);
	std::swap(capacity_, integer.capacity_);
	std::swap(sign_, integer.sign_);
}

void bigint::reserve(std::size_t new_capacity)
{
	if (new_capacity > capacity_)
	{
		block_type* const old_data = data_;

		data_ = reinterpret_cast<block_type*>(std::realloc(data_, new_capacity));
		capacity_ = new_capacity;

		if (!data_)
		{
			std::free(old_data);
			size_ = capacity_ = 0;
			throw std::bad_alloc();
		}
	}
}
void bigint::shrink_to_fit()
{
	block_type* const old_data = data_;

	if (size_)
	{
		data_ = reinterpret_cast<block_type*>(std::realloc(data_, size_));
		capacity_ = size_;

		if (!data_)
		{
			std::free(old_data);
			size_ = capacity_ = 0;
		}
	}
	else
	{
		reset();
	}
}

bigint::size_type bigint::capacity() const noexcept
{
	return capacity_;
}

#ifdef _BIGNUM_HAS_NAMESPACE
}
#endif