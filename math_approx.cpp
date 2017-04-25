// math_approx.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

#include "halfp.h"

void test_number(halfp const& x)
{
    std::cout << static_cast<float>(x) << std::endl;
    std::cout << "reciprocals: " << static_cast<float>(x.rcp()) << ", " <<
        static_cast<float>(x.rcp(2)) << ", " <<
        static_cast<float>(x.rcp(3)) << ", " <<
        static_cast<float>(halfp(1 / static_cast<float>(x))) << std::endl;

    std::cout << "reciprocal square roots: " << static_cast<float>(x.rsqrt()) << ", " <<
        static_cast<float>(x.rsqrt(2)) << ", " <<
        static_cast<float>(x.rsqrt(3)) << ", " <<
        static_cast<float>(halfp(1 / sqrt(static_cast<float>(x)))) << std::endl;

    std::cout << "round: " << x.round() << std::endl;
    std::cout << "fixed point 4: " << (x.round(4) >> 4) << '.' << ((x.round(4) & 15) / 16.f)*10000 << std::endl;

    std::cout << "exp2: " << x.exp2() << std::endl;
    std::cout << "log2: " << x.log2() << std::endl;

    std::cout << std::endl;
}

int main()
{
    float nums[] = { 0.45123f, 19.560047f , 13.54389573984f, 17.f, 5, 5.6f };

    for (auto num : nums)
        test_number(num);

    return 0;
}

