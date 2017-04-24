#include <intrin.h>
#include <cassert>

#include "halfp.h"

halfp::halfp(float x)
{
    assign(x);
}

halfp::halfp(halfp const & src)
{
    m_data.raw = src.m_data.raw;
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
    // incorrect for numbers larger than 2^10
    halfp tmp(static_cast<float>(1 << (10 - fixed_point)) + static_cast<float>(*this));
    short res = tmp.m_data.raw & ((1 << 10) - 1);
    short sign = tmp.m_data.signed_raw >> 15;
    return (res ^ sign) - sign;
#elif 0
    short res = ((m_data.signed_raw >> 10) & 31) - 15;
    short sign = m_data.signed_raw >> 15;
    res = (1 << (res + fixed_point)) + ((m_data.raw & ((1 << 10) - 1)) >> (10 - res - fixed_point));

    return (res ^ sign) - sign;
#else
    unsigned short res = ((m_data.signed_raw >> 10) & 31) - 15;
    res = (1 << (res + fixed_point)) + ((m_data.raw & ((1 << 10) - 1)) >> (10 - res - fixed_point));

    return res;
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
    dst += 15;
    dst *= 1 << 10;
    dst.m_data.signed_raw = dst.round();

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
