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
#include <stdexcept>

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
		sign_ = false;
		throw std::bad_alloc();
	}

	std::copy(integer.data_, integer.data_ + integer.capacity_, data_);
}
bigint::bigint(const bigint& integer, size_type new_capacity)
	: capacity_(new_capacity), sign_(integer.sign_)
{
	if (capacity_ < integer.capacity_)
	{
		capacity_ = 0;
		sign_ = false;
		throw std::invalid_argument("new_capacity < integer.capacity()");
	}

	data_ = reinterpret_cast<block_type*>(std::calloc(capacity_, sizeof(block_type)));

	if (!data_)
	{
		capacity_ = 0;
		sign_ = false;
		throw std::bad_alloc();
	}

	std::copy(integer.data_, integer.data_ + integer.capacity_, data_);
	std::fill(data_ + integer.capacity_, data_ + capacity_, 0);
}
bigint::bigint(bigint&& integer) noexcept
	: data_(integer.data_), capacity_(integer.capacity_), sign_(integer.sign_)
{
	integer.data_ = nullptr;
	integer.capacity_ = 0;
	integer.sign_ = false;
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
		const size_type old_capacity = capacity_;

		data_ = reinterpret_cast<block_type*>(std::realloc(data_, sizeof(block_type) * integer.capacity_));
		capacity_ = integer.capacity_;

		if (!data_)
		{
			data_ = old_data;
			capacity_ = old_capacity;
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
	integer.sign_ = false;

	return *this;
}
bool bigint::operator==(const bigint& integer) const noexcept
{
	if (this == &integer) return true;
	if (data_ == integer.data_ && data_ == nullptr) return true;

	const size_type min_capacity = std::min(capacity_, integer.capacity_);

	for (size_type i = 0; i < min_capacity; ++i)
	{
		if (data_[i] != integer.data_[i]) return false;
	}

	const size_type other_capacity = std::max(capacity_, integer.capacity_) - min_capacity;
	block_type* other_data = capacity_ > integer.capacity_ ? data_ : integer.data_;
	block_type* const other_data_end = other_data + other_capacity;

	for (; other_data < other_data_end; ++other_data)
	{
		if (*other_data != 0) return false;
	}

	return sign_ == integer.sign_;
}
bool bigint::operator!=(const bigint& integer) const noexcept
{
	if (this == &integer) return false;
	if (data_ == integer.data_ && data_ == nullptr) return false;

	const size_type min_capacity = std::min(capacity_, integer.capacity_);

	for (size_type i = 0; i < min_capacity; ++i)
	{
		if (data_[i] == integer.data_[i]) return false;
	}

	const size_type other_capacity = std::max(capacity_, integer.capacity_) - min_capacity;
	block_type* other_data = capacity_ > integer.capacity_ ? data_ : integer.data_;
	block_type* const other_data_end = other_data + other_capacity;

	for (; other_data < other_data_end; ++other_data)
	{
		if (*other_data != 0) return true;
	}

	return sign_ != integer.sign_;
}
bigint& bigint::operator+=(const bigint& integer)
{
	if (sign_ != integer.sign_)
	{
		// TODO: sub_unsigned
	}
	else
	{
		add_unsigned(integer);
	}

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

void bigint::reserve(size_type new_capacity)
{
	if (new_capacity > capacity_)
	{
		block_type* const old_data = data_;
		const size_type old_capacity = capacity_;

		data_ = reinterpret_cast<block_type*>(std::realloc(data_, sizeof(block_type) * new_capacity));
		capacity_ = new_capacity;

		if (!data_)
		{
			data_ = old_data;
			capacity_ = old_capacity;
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
		const size_type old_capacity = capacity_;

		for (size_type i = capacity_ - 1; i >= 0; --i)
		{
			if (data_[i] != 0 && i + 1 != capacity_)
			{
				if (!(data_ = reinterpret_cast<block_type*>(std::realloc(data_, capacity_ = i + 1))))
				{
					data_ = old_data;
					capacity_ = old_capacity;
					throw std::bad_alloc();
				}

				return;
			}

			if (i == 0)
			{
				std::free(data_);
				data_ = nullptr;
				capacity_ = 0;
				sign_ = false;

				return;
			}
		}
	}
}

void bigint::add_unsigned(const bigint& integer)
{
	const bigint* smaller;
	const bigint* larger;
	if (capacity_ > integer.capacity_)
	{
		larger = this;
		smaller = &integer;
	}
	else
	{
		larger = &integer;
		smaller = this;
	}

	const size_t limit = smaller->capacity_;
	bool carry = false;
	block_type preserved;
	for (size_t i = 0; i < limit; ++i)
	{
		preserved = smaller->data_[i];
		data_[i] = preserved + larger->data_[i];
		carry = (data_[i] < preserved) || !(larger->data_[i] + 1);
	}

	if (carry)
	{
		size_t carry_end = limit;
		while (!(larger->data_[carry_end++] + 1));

		const bool carry_exceeds = larger->capacity_ < carry_end;
		size_t new_capacity = larger->capacity_ + carry_exceeds;
		block_type* const old_data = data_;
		data_ = reinterpret_cast<block_type*>(std::realloc(data_, sizeof(block_type) * new_capacity));
		if (!data_)
		{
			data_ = old_data;
			throw std::bad_alloc();
		}
		capacity_ = new_capacity;

		std::fill(data_ + limit, data_ + carry_end, 0);
		if (carry_exceeds)
		{
			data_[carry_end] = 1;
		}
		else
		{
			if (this != larger)
			{
				std::copy(data_ + carry_end, data_ + larger->capacity_, larger->data_);
			}
			++data_[carry_end];
		}
	}
	else if (capacity_ < larger->capacity_)
	{
		block_type* const old_data = data_;
		data_ = reinterpret_cast<block_type*>(std::realloc(data_, sizeof(block_type) * larger->capacity_));
		if (!data_)
		{
			data_ = old_data;
			throw std::bad_alloc();
		}
		capacity_ = larger->capacity_;
		std::copy(data_ + limit, data_ + capacity_, larger->data_);
	}
}

bigint::size_type bigint::capacity() const noexcept
{
	return capacity_;
}

#ifdef _BIGNUM_HAS_NAMESPACE
}
#endif