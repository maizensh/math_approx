#include <intrin.h>
#include <cassert>
#include <algorithm>

#include "halfp.h"

halfp::halfp(float x)
{
    assign(x);
}

halfp::halfp(halfp const & src)
{
    m_data.raw = src.m_data.raw;
}

halfp::halfp(data data)
{
    m_data = data;
}

halfp::halfp(unsigned short num)
{
#if 0
    halfp tmp0;
    tmp0.m_data.raw = (15 + 10) << 10;
    tmp0.m_data.raw |= num & ((1 << 10) - 1);
    tmp0 -= halfp(static_cast<float>(1 << 10));

#if 0
    // this needs to be fixed
    halfp tmp1;
    tmp1.m_data.raw = (15 + 15) << 10;
    tmp1.m_data.raw |= (num >> 10) << 5;

    tmp0 += tmp1;
#endif
#else
    halfp tmp0(static_cast<float>(num));
#endif
    m_data = tmp0.m_data;
}

halfp::operator float() const
{
    singlep x;

    x.raw = (m_data.raw & ~(1 << 15)) << (23 - 10);
    x.raw += (127 - 15) << 23;
    x.raw |= (m_data.raw & (1 << 15)) << 16;

    return x.x;
}

halfp& halfp::operator=(halfp const & src)
{
    m_data.raw = src.m_data.raw;
    return *this;
}

halfp & halfp::operator=(float x)
{
    assign(x);
    return *this;
}

halfp & halfp::operator*=(halfp const& x)
{
    assign(static_cast<float>(*this) * static_cast<float>(x));
    return *this;
}

halfp & halfp::operator-=(halfp const & x)
{
    assign(static_cast<float>(*this) - static_cast<float>(x));
    return *this;
}

halfp halfp::abs() const
{
    halfp dst(*this);
    dst.m_data.raw &= (1 << 15) - 1;
    return dst;
}

halfp & halfp::operator+=(halfp const & x)
{
    assign(static_cast<float>(*this) + static_cast<float>(x));
    return *this;
}

halfp halfp::operator-() const
{
    halfp dst(*this);

    dst.m_data.raw ^= 1 << 15;
    return dst;
}

unsigned short halfp::round(int fixed_point) const
{
#if 0
    // this works, but is it worth it?
    unsigned short res = ((m_data.signed_raw >> 10) & 31) - 15;
    unsigned short big = 1 << (res + fixed_point);
    // this depends on ALU implementation, if there are no signed shifts then this will work correctly
    // if the power is non zero then one will always evaluate to zero, otherwise the extra summation will be subtracted
    unsigned short small0 = (m_data.raw & ((1 << 10) - 1)) >> (10 - res - fixed_point);
    unsigned short small1 = (m_data.raw & ((1 << 10) - 1)) << (res + fixed_point - 10);
    unsigned short carry = ((m_data.raw & ((1 << 10) - 1)) >> (10 - res - fixed_point - 1)) & 1;
    res = big + small0 + small1 + carry - (small0 & small1);
    return res;
#elif 0
    // incorrect for numbers larger than 2^10, but fast otherwise
    halfp tmp(static_cast<float>(1 << (10 - fixed_point)) + static_cast<float>(this->abs()));
    short res = tmp.m_data.raw & ((1 << 10) - 1);
    short sign = m_data.signed_raw >> 15;
    return (res ^ sign) - sign;
#elif 0
    short res = ((m_data.signed_raw >> 10) & 31) - 15;
    short sign = m_data.signed_raw >> 15;
    res = (1 << (res + fixed_point)) + ((m_data.raw & ((1 << 10) - 1)) >> (10 - res - fixed_point));

    return (res ^ sign) - sign;
#elif 0
    halfp tmp0(this->abs()), tmp1;
    short exp_diff = ((tmp0.m_data.raw >> 10) & 31) - 15 - 10;
    exp_diff = std::max(exp_diff, 0i16);
    tmp1.m_data.raw = exp_diff & (1 << 15);
    tmp1.m_data.signed_raw >>= exp_diff; // replicate bits for mantissa
    unsigned short big = tmp1.m_data.raw;
    tmp1.m_data.raw >>= 5; // move into place
    tmp1.m_data.raw += 15 << 10; // add bias
    tmp0 -= tmp1; // this one can be safely rounded
    
    tmp0 += static_cast<float>(1 << (10 - fixed_point));
    unsigned short res = tmp0.m_data.raw & ((1 << 10) - 1);
    res += big;
    short sign = m_data.signed_raw >> 15;
    return (res ^ sign) - sign;
#else
    return static_cast<unsigned short>(::round(static_cast<float>(*this)));
#endif
}

halfp halfp::rcp(int order) const
{
    halfp dst;

#if 1
    dst.m_data.raw = ((15 * 2 - 1) << 10) | 0x3A8;
#else
    dst.m_data.mantissa = 936;
    dst.m_data.exponent = (15 * 2 - 1) << 10;
#endif
    dst.m_data.raw -= m_data.raw & ~(1 << 15);
    dst.m_data.raw |= m_data.raw & (1 << 15);

    halfp tmp0(dst), tmp1;

    switch (order) {
    case 0:
    case 1:
        break;
    case 2:
        tmp0 *= *this;
        tmp0 -= 2.f;
        dst *= -tmp0;
        break;
    case 3:
        // unstable for half precision
        tmp0 *= *this;
        tmp1 = tmp0;
        tmp0 -= 3.f;
        tmp0 *= tmp1;
        tmp0 += 3.f;
        dst *= tmp0;
        break;
    default:
        assert(false);
    };

    return dst;
}

halfp halfp::rsqrt(int order) const
{
    halfp dst;

#if 1
    dst.m_data.raw = (22 << 10) | 0x1BE;
#else
    dst.m_data.mantissa = 936;
    dst.m_data.exponent = (15 * 2 - 1) << 10;
#endif
    dst.m_data.raw -= (m_data.raw & ~(1 << 15)) >> 1;
    dst.m_data.raw |= m_data.raw & (1 << 15);

    halfp tmp0(dst), tmp1;

    switch (order) {
    case 0:
    case 1:
        break;
    case 2:
        tmp0 *= dst;
        tmp0 *= *this;
        tmp0 -= 3.f;
        dst *= -tmp0;
        dst.m_data.raw -= 1 << 10;
        break;
    case 3:
        // doesn't converge exactly due to lack of precision
        // but seems stable
        tmp0 *= dst;
        tmp0 *= *this;
        tmp1 = tmp0;
        tmp0 *= 0.375f;
        tmp0 -= 1.25f;
        tmp0 *= tmp1;
        tmp0 += 1.875f;
        dst *= tmp0;
        break;
    default:
        assert(false);
    };

    return dst;
}

halfp halfp::exp2() const
{
    halfp dst(*this);
    dst += 15.f;

    dst *= static_cast<float>(1 << 10);
    dst.m_data.signed_raw = dst.round();

    return dst;
}

halfp halfp::log2() const
{
    halfp dst(m_data.raw);
    dst *= static_cast<float>(1.f / (1 << 10));
    dst -= 15.f;

    return dst;
}

void halfp::assign(float x)
{
    singlep y;
    y.x = x;

#if 1
    y.raw -= (127 - 15) << 23;
    m_data.raw = (y.raw >> (23 - 10 - 1)) & 1;
    m_data.raw += y.raw >> (23 - 10);
    m_data.raw |= _bittest(&y.intrin_raw, 31) << 15;
#else
    m_data.sign = y.sign;
    m_data.mantissa = (y.mantissa >> (23 - 10)) + ((y.mantissa >> (23 - 10 - 1)) & 1);
    m_data.exponent = y.exponent - 127 + 15;
#endif
}
