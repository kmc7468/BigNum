#ifndef _BIGNUM_HPP
#define _BIGNUM_HPP

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
///// Global Macros
/////////////////////////////////////////////////////////////////

#define _BIGNUM_MAJOR 1ull
#define _BIGNUM_MINOR 0ull

#ifdef _BIGNUM_HAS_NAMESPACE
#	define _BIGNUM_DETAILS details
#else
#	define _BIGNUM_DETAILS _BIGNUM_DETAILS
#endif
#define _BIGNUM_DETAILS_BEGIN namespace _BIGNUM_DETAILS {
#define _BIGNUM_DETAILS_END }

/////////////////////////////////////////////////////////////////
///// Includes
/////////////////////////////////////////////////////////////////

#include <cstddef>
#include <cstdint>

/////////////////////////////////////////////////////////////////
///// Declarations
/////////////////////////////////////////////////////////////////

#ifdef _BIGNUM_HAS_NAMESPACE
namespace _BIGNUM_HAS_NAMESPACE
{
#endif

class bigint
{
public:
	using block_type = std::uint32_t;
	using size_type = std::size_t;

public:
	bigint() noexcept = default;
	bigint(std::int32_t integer);
	bigint(std::uint32_t integer);
	bigint(std::int64_t integer);
	bigint(std::uint64_t integer);
	bigint(const bigint& integer);
	bigint(const bigint& integer, size_type new_capacity);
	bigint(bigint&& integer) noexcept;
	~bigint();

public:
	bigint& operator=(const bigint& integer);
	bigint& operator=(bigint&& integer) noexcept;
	bool operator==(const bigint& integer) const noexcept;
	bool operator!=(const bigint& integer) const noexcept;
	bool operator>(const bigint& integer) const noexcept;
	bool operator>=(const bigint& integer) const noexcept;
	bool operator<(const bigint& integer) const noexcept;
	bool operator<=(const bigint& integer) const noexcept;
	bigint operator+(const bigint& integer) const;
	bigint& operator+=(const bigint& integer);
	bigint& operator++();
	bigint operator++(int);
	bigint operator-(const bigint& integer) const;
	bigint& operator-=(const bigint& integer);
	bool operator!() const noexcept;
	explicit operator bool() const noexcept;

public:
	void reset() noexcept;
	void swap(bigint& integer) noexcept;

	void reserve(size_type new_capacity);
	void shrink_to_fit();

	bool zero() const noexcept;
	bool positive() const noexcept;
	bool negative() const noexcept;

private:
	void add_unsigned_(const bigint& integer);
	void sub_unsigned_(const bigint& integer);

public:
	const block_type* data() const noexcept;
	block_type* data() noexcept;
	size_type capacity() const noexcept;
	bool sign() const noexcept;

private:
	block_type* data_ = nullptr;
	size_type capacity_ = 0;
	bool sign_ = false;
};

#ifdef _BIGNUM_HAS_NAMESPACE
}
#endif

#endif