
#include <math_tools.hpp>

#include <gtest/gtest.h>
#include <iostream>

using namespace math;

TEST(polynomial, test) { EXPECT_EQ(true, 1) << "a"; }

TEST(polynomial, test_construction) {
    std::array<complex, 3> pow2 = {2, 3, 4};
    const Polynomial p1(pow2);
    EXPECT_EQ(p1.degree(), 2) << "Polynomial degree should be 2";
    EXPECT_EQ(p1.at(1).real(), 3) << "Polynomial at() failed";
    EXPECT_ANY_THROW(p1.at(3)) << "at() should throw";
    EXPECT_EQ(p1[2].real(), 4) << "operator[]";
    EXPECT_EQ(p1[3].real(), 0) << "operator[] default";

    std::array<complex, 4> pow1 = {2, 3, 0, 0};
    const Polynomial p2(pow1);
    EXPECT_EQ(p2.degree(), 1);
}

TEST(polynomial, proxy_access) {
    std::array<complex, 3> pow2 = {2, 3, 4};
    Polynomial p1(pow2);

    complex two{2, 0};
    bool b = p1[0] == two;
    EXPECT_TRUE(b) << "proxy operator== failed";

    complex v = p1[0];
    EXPECT_EQ(v, two) << "operator[] returning proxy accesses wrong element";

    p1[0] = complex(2, 3);
    EXPECT_EQ(complex(p1[0]), complex(2, 3))
        << "Proxy assignment failed when degree is not changed";
    p1[5] = complex(43);
    EXPECT_EQ(p1.degree(), 5) << "Proxy assignment failed to change degree";
    EXPECT_EQ(complex(p1[5]), complex(43))
        << "Proxy assignment failed when changing degree";
}

testing::AssertionResult
test_complex_near(complex const& a, complex const& b,
                  double max_error = std::numeric_limits<double>::epsilon()) {
    double d = std::abs(a) - std::abs(b);
    if (d < max_error) return testing::AssertionSuccess();
    return testing::AssertionFailure()
        << "abs(a) - abs(b) = " << d << ", should be less than " << max_error;
}

TEST(polynomial, call) {
    Polynomial p(std::array<complex, 5>{0, -1, 0, 0, 1});
    EXPECT_TRUE(test_complex_near(p(complex(1)), complex(0)))
        << "x^4-x at x=1 does not equal 0";
    EXPECT_TRUE(test_complex_near(p(complex(0, 1)), complex(1, -1)))
        << "x^4-x at x=i does not equal 1-i";
}

TEST(polynomial, solve_simple) {
    Polynomial dg0(std::to_array<complex>({4}));
    Polynomial dg1(std::to_array<complex>({
        {4, 5 },
        {3, -6}
    }));
    Polynomial dg2(std::to_array<complex>({1, 0, 1}));
    std::vector<complex> roots0 = {};
    std::vector<complex> roots1 = {
        {0.4, -13.0 / 15.0}
    };
    std::vector<complex> roots2 = {
        {0, 1 },
        {0, -1}
    };

    EXPECT_EQ(roots0, find_roots(dg0));
    EXPECT_EQ(roots1, find_roots(dg1));
    EXPECT_EQ(roots2, find_roots(dg2));
}

testing::AssertionResult test_roots_near(std::vector<complex> const& r1,
                                         std::vector<complex> const& r2,
                                         double tol) {
    std::vector<complex> notfound;
    for (auto& root : r1) {
        // There must be a root in r2 that is near
        bool found = false;
        for (size_t i = 0; i < r2.size(); ++i) {
            if (std::abs(root) - std::abs(r2[i]) < tol) {
                found = true;
                break;
            }
        }
        if (!found) notfound.push_back(root);
    }
    if (notfound.empty()) return testing::AssertionSuccess();
    auto f = testing::AssertionFailure();

    f << "Roots [";
    for (auto const& r : notfound) f << r << ", ";
    f << "] did not match any precomputed root";

    return f;
}

TEST(polynomial, find_roots) {
    Polynomial deg4(std::to_array<complex>({-1, -1, 0, 1}));
    std::vector<complex> roots4 = {
        {1.3247,   0       },
        {-0.66236, -0.56228},
        {-0.66236, +0.56228}
    };

    EXPECT_TRUE(test_roots_near(find_roots(deg4), roots4, 0.0001));

    Polynomial deg3(std::to_array<complex>({-1, 0, 0, 1}));
    std::vector<complex> roots3 = {
        {1,        0                    },
        {-1.0 / 2, std::sqrt(3.0) / 2.0 },
        {-1.0 / 2, -std::sqrt(3.0) / 2.0},
    };
    EXPECT_TRUE(test_roots_near(find_roots(deg3), roots3, 0.0001));
}

TEST(polynomial, derivative) {
    // f = 3x^2 + 4x - 5
    // f' = 6x + 4
    Polynomial p(std::to_array<complex>({-5, 4, 3}));
    Polynomial dv(std::to_array<complex>({4, 6}));
    EXPECT_EQ(dv, derivative(p)) << "derivative of polynomial failed";
}

TEST(polynomial, print) {
    std::cout << Polynomial(std::to_array<complex>({-5, 4, 3})) << "\n";
    EXPECT_TRUE(true);
}
