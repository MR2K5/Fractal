#include <math_tools.hpp>

#include <cassert>
#include <ranges>
#include <utility>
#include <ostream>

namespace math {

int Polynomial::degree() const noexcept { return coeffs.size() - 1; }

complex Polynomial::operator[](int i) const noexcept {
    assert(i >= 0 && "negative index");
    if (i > degree()) return complex{0.0};
    return coeffs[i];
}

complex Polynomial::at(int i) const {
    if (i > degree() || i < 0)
        throw std::out_of_range("polynomial has no term of degree "
                                + std::to_string(i));
    return coeffs[i];
}

Polynomial::Polynomial(std::span<complex const> coeffs_)
    : coeffs(coeffs_.begin(), coeffs_.end()) {
    erase_trailing();
}

complex Polynomial::operator()(complex const& z) const noexcept {
    complex result{};
    for (int i = 0; i < coeffs.size(); ++i) {
        result += coeffs[i] * std::pow(z, i);
    }
    return result;
}

Polynomial& Polynomial::operator/=(complex const& v) {
    for (auto& c : coeffs) c /= v;
    return *this;
}

void Polynomial::erase_trailing() noexcept {
    auto last = std::ranges::find_if(
        coeffs.rbegin(), coeffs.rend(),
        [](complex const& v) { return v.imag() != 0 || v.real() != 0; });

    coeffs.erase(last.base(), coeffs.end());
}

Polynomial::Proxy::operator complex() && noexcept {
    return std::as_const(*pl)[idx];
}

void Polynomial::Proxy::operator=(complex const& nw) && {
    if (nw == complex{0.0}) {
        if (idx <= pl->degree()) {
            pl->coeffs[idx] = nw;
            if (idx == pl->degree()) pl->erase_trailing();
        }
    } else {
        if (idx > pl->degree()) { pl->coeffs.resize(idx + 1); }
        pl->coeffs[idx] = nw;
    }
}

Polynomial::Proxy::Proxy(Polynomial* p, int i): pl(p), idx(i) {}

auto Polynomial::operator[](int i) noexcept -> Proxy {
    assert(i >= 0 && "negative index");
    return Proxy(this, i);
}

std::ostream& operator<<(std::ostream& os, Polynomial const& p) {
    for (int i = p.degree(); i > -1; --i) {
        os << p[i] << "";
        if (i >= 2) os << "*z^" << i;
        if (i == 1) os << "*z";
    }
    return os;
}

namespace {
std::vector<complex> durand_kerner_step(Polynomial const& p,
                                        std::vector<complex> const& guess) {
    std::vector tmp = guess;

    for (size_t i = 0; i < tmp.size(); ++i) {
        complex denom{1, 0};
        const complex c = guess[i];
        for (size_t j = 0; j < tmp.size(); ++j) {
            if (i == j) continue;
            denom *= (c - guess[j]);
        }

        tmp[i] = guess[i] - p(guess[i]) / denom;
    }
    return tmp;
}
}  // namespace

std::vector<complex> find_roots(Polynomial const& pl, double tolerance) {
    if (pl.degree() == 0) return {};
    if (pl.degree() == 1) return {-pl[0] / pl[1]};
    if (pl.degree() == 2) {
        // solve 2nd degree polynomial
        const complex a = pl[2], b = pl[1], c = pl[0];
        complex z1 = (-b + std::sqrt(b * b - 4.0 * a * c)) / (2.0 * a);
        complex z2 = (-b - std::sqrt(b * b - 4.0 * a * c)) / (2.0 * a);
        return {z1, z2};
    }

    Polynomial p(pl);
    p /= pl[pl.degree()];

    std::vector<complex> guess(pl.degree());

    constexpr complex initial = {0.4345, 0.90234};
    for (size_t i = 0; i < guess.size(); ++i) {
        guess[i] = std::pow(initial, i + 1);
    }

    auto test_roots_near = [tolerance](std::vector<complex> const& a, std::vector<complex> const& b){
        assert(a.size() == b.size());
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::norm(a[i]) - std::norm(b[i]) > tolerance) return false;
        }
        return true;
    };

    std::vector<complex> new_guess = guess;
    do {
        guess = new_guess;
        new_guess = durand_kerner_step(p, guess);
    } while (!test_roots_near(guess, new_guess));

    return guess;
}

Polynomial derivative(const Polynomial &p) {
    Polynomial res{{}};
    for (int i = p.degree(); i > 0; --i) {
        res[i - 1] = p[i] * static_cast<double>(i);
    }
    return res;
}

}  // namespace math
