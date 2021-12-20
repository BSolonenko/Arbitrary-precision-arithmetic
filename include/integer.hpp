#pragma once
#include <vector>
#include <string>
#include <ostream>
#include <istream>

class Integer
{
public:
	using Digit = unsigned long long;

	Integer() = default;
	explicit Integer(const std::string& num);

	bool IsZero() const;
	bool IsNaN() const;

	size_t BitDepth() const;

	Digit operator [] (const size_t index) const;

	void operator += (const Integer& other);
	void operator -= (Integer other);
	void operator <<= (const Digit& shift);
	void operator >>= (const Digit& shift);

	Integer operator + (const Integer& other) const;
	Integer operator - (const Integer& other) const;
	Integer operator << (const Digit& shift) const;
	Integer operator >> (const Digit& shift) const;

	friend std::ostream& operator << (std::ostream& os, Integer num);
	friend std::istream& operator >> (std::istream& os, Integer& num);

private:
	void Complement();
	void RemoveLeadingZeros();

	std::vector<Digit> m_number{ 0 };
	bool m_positive{ true };
	bool m_nan{ false };
};
