#pragma once

#include <complex>
#include <span>
#include <utility>
#include <vector>
#include <iosfwd>

namespace math {

using complex_function = std::complex<double>(std::complex<double> const&);
using complex          = std::complex<double>;

class Polynomial {
    struct Proxy {
        operator complex() && noexcept;
        void operator=(complex const& nw) &&;

        Proxy(Polynomial* p, int i);
        Proxy(Proxy const&)            = delete;
        Proxy& operator=(Proxy const&) = delete;

        friend bool operator==(Proxy const&& p, complex const& v) noexcept {
            return (std::as_const(*p.pl))[p.idx] == v;
        }

    private:
        Polynomial* pl;
        int idx;
    };

public:
    int degree() const noexcept;
    complex operator[](int i) const noexcept;
    complex at(int i) const;
    Proxy operator[](int i) noexcept;

    explicit Polynomial(std::span<complex const> coeffs_);
    Polynomial() = default;

    complex operator()(complex const& z) const noexcept;

    Polynomial& operator/=(complex const& v);
    /// Multiply this polynomial by (x - x0)
    Polynomial multiply_by_term(complex const& x0) const;

    static Polynomial from_roots(std::span<complex const> roots);
    static Polynomial one();

    friend bool operator==(Polynomial const& a, Polynomial const& b) = default;
    friend std::ostream& operator<<(std::ostream& os, Polynomial const& p);

private:
    std::vector<complex> coeffs;

    void erase_trailing() noexcept;
};

std::vector<complex> find_roots(Polynomial const& pl, double tolerance = std::numeric_limits<double>::epsilon());

Polynomial derivative(Polynomial const& p);

}  // namespace math
