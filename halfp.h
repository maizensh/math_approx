#pragma once

// scalar
class halfp {
public:
    halfp(float x = 0.f);
    halfp(halfp const& src);
    halfp(unsigned short num);

    operator float() const;
    halfp& operator=(halfp const& src);
    halfp& operator=(float x);

    halfp& operator*=(halfp const& x);
    halfp& operator-=(halfp const& x);
    halfp& operator+=(halfp const& x);
    halfp operator-() const;

    halfp abs() const;
    unsigned short round(int fixed_point = 0) const;

    halfp rcp(int oder = 0) const;
    halfp rsqrt(int order = 0) const;

    halfp exp2() const;
    halfp log2() const;

private:
    void assign(float x);

    union data {
        unsigned short raw;
        short signed_raw;
        struct {
            unsigned mantissa : 10;
            unsigned exponent : 5;
            unsigned sign : 1;
        };
    } m_data;
    halfp(data data);

    union singlep {
        float x;
        unsigned raw;
        long intrin_raw;
        struct {
            unsigned mantissa : 23;
            unsigned exponent : 8;
            unsigned sign : 1;
        };
    };
};
