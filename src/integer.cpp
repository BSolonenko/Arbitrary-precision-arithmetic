#include "integer.hpp"
#include <algorithm>

namespace 
{
	constexpr Integer::Digit BASE = 1000000000;
	constexpr Integer::Digit COMPLEMENT = BASE - 1;
	constexpr Integer::Digit BASE_BITNESS = 9;

	void bin_multiplication(Integer a, Integer::Digit b, Integer& res)
	{
		while (b)
		{
			if (b & 1)
			{
				res += a;
			}
			a += a;
			b >>= 1;
		}
	}
}

Integer::Integer(const std::string& num) : m_number()
{
	if (num.empty())
	{
		m_nan = true;
		return;
	}

	auto begin = num.begin();
	if (*begin == '-' || *begin == '+')
	{
		m_positive = *begin == '+';
		++begin;
	}

	if (begin == num.end() || !std::all_of(begin, num.end(), std::isdigit))
	{
		m_nan = true;
		return;
	}

	m_number.reserve(num.size() / BASE_BITNESS + 1);

	const int first = num[0] == '-' || num[0] == '+' ? 1 : 0;
	for (int i = num.size(); i > first; i -= BASE_BITNESS)
	{
		const std::string digitStr(num.begin() + std::max(i - static_cast<int>(BASE_BITNESS), first), num.begin() + i);
		m_number.push_back(std::stoull(digitStr));
	}

	if (!m_positive)
	{
		Complement();
	}

	RemoveLeadingZeros();
}

bool Integer::IsZero() const
{
	return !IsNaN() && BitDepth() == 1 && m_number[0] == 0;
}

bool Integer::IsNaN() const
{
	return m_nan;
}

size_t Integer::BitDepth() const
{
	return m_number.size();
}

Integer::Digit Integer::operator [] (const size_t index) const
{
	if (index < BitDepth())
	{
		return m_number[index];
	}
	else
	{
		return m_positive ? 0 : COMPLEMENT;
	}
}

void Integer::operator += (const Integer& other)
{
	if (other.m_nan || m_nan)
	{
		m_nan = true;
		m_number.clear();
		return;
	}

	const size_t maxSize = std::max(BitDepth(), other.BitDepth());
	m_number.resize(maxSize + 2, m_positive ? 0 : COMPLEMENT);

	Digit carry = 0;
	for (size_t i = 0; i < BitDepth(); ++i)
	{
		m_number[i] += carry + other[i];
		carry = m_number[i] / BASE;
		m_number[i] %= BASE;
	}

	m_positive = m_number.back() == 0;

	RemoveLeadingZeros();
}

void Integer::operator -= (Integer other)
{
	other.Complement();
	other.m_positive = !other.m_positive;
	*this += other;
}

void Integer::operator *= (Integer other)
{
	if (IsNaN() || other.IsNaN())
	{
		m_number.clear();
		m_nan = true;
		return;
	}

	if (IsZero() || other.IsZero())
	{
		m_number.resize(1);
		m_number[0] = 0;
		return;
	}
	
	Integer res;
	res.m_positive = m_positive == other.m_positive;

	if (!m_positive)
	{
		Complement();
		m_positive = true;
	}

	if (!other.m_positive)
	{
		other.Complement();
		other.m_positive = true;
	}

	for (size_t i = 0; i < other.BitDepth(); ++i)
	{
		bin_multiplication(*this, other[i], res);
		if (i < other.BitDepth() - 1)
		{
			*this <<= 1;
		}
	}

	if (!res.m_positive)
	{
		res.Complement();
	}
	*this = std::move(res);
}

void Integer::operator <<= (const Digit& shift)
{
	if (shift == 0 || IsNaN() || IsZero())
	{
		return;
	}
	m_number.resize(BitDepth() + shift, 0);

	for(size_t i = BitDepth() - 1; i >= shift; --i)
	{
		std::swap(m_number[i], m_number[i - shift]);
	}
}

void Integer::operator >>= (const Digit& shift)
{
	if (shift == 0 || IsNaN() || IsZero())
	{
		return;
	}

	if (shift >= BitDepth())
	{
		m_number.resize(1);
		m_number[0] = 0;
		m_positive = true;
		return;
	}

	m_number.erase(m_number.begin(), m_number.begin() + shift);
}

Integer Integer::operator + (const Integer& other) const
{
	Integer res(*this);
	res += other;
	return res;
}

Integer Integer::operator - (const Integer& other) const
{
	Integer res(*this);
	res -= other;
	return res;
}

Integer Integer::operator * (const Integer& other) const
{
	Integer res(*this);
	res *= other;
	return res;
}

Integer Integer::operator << (const Digit& shift) const
{
	Integer result(*this);
	result <<= shift;
	return result;
}

Integer Integer::operator >> (const Digit& shift) const
{
	Integer result(*this);
	result >>= shift;
	return result;
}

std::ostream& operator << (std::ostream& os, Integer num)
{
	if (num.IsNaN())
	{
		os << "nan";
		return os;
	}

	if (num.IsZero())
	{
		os << '0';
		return os;
	}

	if (!num.m_positive)
	{
		num.Complement();
		os << '-';
	}

	for (auto digitItr = num.m_number.rbegin(); digitItr < num.m_number.rend(); ++digitItr)
	{
		std::string tmp = std::to_string(*digitItr);
		if (tmp.size() < BASE_BITNESS && digitItr != num.m_number.rbegin())
		{
			os << std::string(BASE_BITNESS - tmp.size(), '0');
		}
		os << tmp;
	}

	return os;
}

std::istream& operator >> (std::istream& os, Integer& num)
{
	std::string num_str;
	os >> num_str;
	num = Integer(num_str);
	return os;
}

void Integer::Complement()
{
	Digit carry = 1;
	for (auto& digit : m_number)
	{
		digit = COMPLEMENT - digit + carry;
		carry = digit / BASE;
		digit %= BASE;
	}
}

void Integer::RemoveLeadingZeros()
{
	if (m_positive)
	{
		while (BitDepth() > 1 && m_number.back() == 0)
		{
			m_number.pop_back();
		}
	}
	else
	{
		while (BitDepth() > 1 && m_number[BitDepth() - 1] == COMPLEMENT && m_number[BitDepth() - 2] != 0)
		{
			m_number.pop_back();
		}
	}

	m_positive |= IsZero();
}
