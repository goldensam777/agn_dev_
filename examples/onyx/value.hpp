#pragma once

#include <cmath>
#include <complex>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace onyx {

struct Quaternion {
    double r{0.0};
    double i{0.0};
    double j{0.0};
    double k{0.0};

    Quaternion() = default;
    Quaternion(double real, double imag_i, double imag_j, double imag_k)
        : r(real), i(imag_i), j(imag_j), k(imag_k) {}

    [[nodiscard]] double norm() const noexcept {
        return std::sqrt(r * r + i * i + j * j + k * k);
    }

    [[nodiscard]] Quaternion conjugate() const noexcept {
        return Quaternion(r, -i, -j, -k);
    }

    // Multiplication non-commutative : i^2 = j^2 = k^2 = -1, ij = k, jk = i, ki = j
    [[nodiscard]] Quaternion operator*(const Quaternion& o) const noexcept {
        return Quaternion(
            r * o.r - i * o.i - j * o.j - k * o.k,
            r * o.i + i * o.r + j * o.k - k * o.j,
            r * o.j - i * o.k + j * o.r + k * o.i,
            r * o.k + i * o.j - j * o.i + k * o.r
        );
    }

    [[nodiscard]] Quaternion operator+(const Quaternion& o) const noexcept {
        return Quaternion(r + o.r, i + o.i, j + o.j, k + o.k);
    }

    [[nodiscard]] Quaternion operator-(const Quaternion& o) const noexcept {
        return Quaternion(r - o.r, i - o.i, j - o.j, k - o.k);
    }

    [[nodiscard]] Quaternion operator/(const Quaternion& o) const {
        double d = o.r * o.r + o.i * o.i + o.j * o.j + o.k * o.k;
        if (d == 0.0) throw std::runtime_error("Division par le quaternion nul");
        Quaternion conj = o.conjugate();
        Quaternion num = (*this) * conj;
        return Quaternion(num.r / d, num.i / d, num.j / d, num.k / d);
    }

    [[nodiscard]] bool operator==(const Quaternion& o) const noexcept {
        return r == o.r && i == o.i && j == o.j && k == o.k;
    }
};

/**
 * @brief Tenseur multidimensionnel Onyx en mémoire contiguë (Zéro Garbage Collector).
 */
class Tensor {
public:
    std::vector<int64_t> shape;
    std::vector<int64_t> strides;
    std::vector<double> data; // Buffer plat contigu en cache

    Tensor() = default;

    explicit Tensor(std::vector<int64_t> s, double default_val = 0.0)
        : shape(std::move(s)) {
        compute_strides();
        size_t total_elements = strides.empty() ? 0 : static_cast<size_t>(shape[0] * strides[0]);
        data.assign(total_elements, default_val);
    }

    Tensor(std::vector<int64_t> s, std::vector<double> elements)
        : shape(std::move(s)), data(std::move(elements)) {
        compute_strides();
    }

    [[nodiscard]] size_t rank() const noexcept {
        return shape.size();
    }

    [[nodiscard]] int64_t length() const {
        if (shape.size() != 1) {
            throw std::runtime_error(".length n'est défini que pour les tenseurs de rang 1");
        }
        return shape[0];
    }

    [[nodiscard]] size_t compute_offset(const std::vector<int64_t>& indices) const {
        if (indices.size() != shape.size()) {
            throw std::runtime_error("Nombre d'indices incorrect pour le tenseur de rang " + std::to_string(shape.size()));
        }
        int64_t offset = 0;
        for (size_t i = 0; i < indices.size(); ++i) {
            if (indices[i] < 0 || indices[i] >= shape[i]) {
                throw std::runtime_error("Index hors limites : " + std::to_string(indices[i]) +
                                         " pour dimension " + std::to_string(shape[i]));
            }
            offset += indices[i] * strides[i];
        }
        return static_cast<size_t>(offset);
    }

    [[nodiscard]] double get(const std::vector<int64_t>& indices) const {
        return data[compute_offset(indices)];
    }

    void set(const std::vector<int64_t>& indices, double value) {
        data[compute_offset(indices)] = value;
    }

private:
    void compute_strides() {
        strides.resize(shape.size());
        if (shape.empty()) return;

        strides.back() = 1;
        for (int i = static_cast<int>(shape.size()) - 2; i >= 0; --i) {
            size_t idx = static_cast<size_t>(i);
            strides[idx] = strides[idx + 1] * shape[idx + 1];
        }
    }
};

enum class ValueKind {
    None,
    Bool,
    Int,
    Real,
    Complex,
    Quaternion,
    String,
    Tensor
};

struct Value {
    ValueKind kind{ValueKind::None};
    bool bool_val{false};
    int64_t int_val{0};
    double real_val{0.0};
    std::complex<double> complex_val{0.0, 0.0};
    Quaternion quat_val;
    std::string str_val;
    Tensor tensor_val;

    Value() = default;
    static Value make_none() { Value v; v.kind = ValueKind::None; return v; }
    static Value make_bool(bool b) { Value v; v.kind = ValueKind::Bool; v.bool_val = b; return v; }
    static Value make_int(int64_t i) { Value v; v.kind = ValueKind::Int; v.int_val = i; return v; }
    static Value make_real(double r) { Value v; v.kind = ValueKind::Real; v.real_val = r; return v; }
    static Value make_complex(double r, double i) { Value v; v.kind = ValueKind::Complex; v.complex_val = {r, i}; return v; }
    static Value make_quaternion(double r, double i, double j, double k) {
        Value v; v.kind = ValueKind::Quaternion; v.quat_val = Quaternion(r, i, j, k); return v;
    }
    static Value make_string(std::string s) { Value v; v.kind = ValueKind::String; v.str_val = std::move(s); return v; }
    static Value make_tensor(Tensor t) { Value v; v.kind = ValueKind::Tensor; v.tensor_val = std::move(t); return v; }

    [[nodiscard]] std::string to_string() const {
        switch (kind) {
            case ValueKind::None: return "none";
            case ValueKind::Bool: return bool_val ? "true" : "false";
            case ValueKind::Int: return std::to_string(int_val);
            case ValueKind::Real: {
                std::ostringstream ss;
                ss << real_val;
                return ss.str();
            }
            case ValueKind::Complex: {
                std::ostringstream ss;
                if (complex_val.real() != 0.0 || complex_val.imag() == 0.0) {
                    ss << complex_val.real();
                    if (complex_val.imag() > 0) ss << " + " << complex_val.imag() << "i";
                    else if (complex_val.imag() < 0) ss << " - " << -complex_val.imag() << "i";
                } else {
                    ss << complex_val.imag() << "i";
                }
                return ss.str();
            }
            case ValueKind::Quaternion: {
                std::ostringstream ss;
                ss << "quaternion(" << quat_val.r << ", " << quat_val.i << ", " << quat_val.j << ", " << quat_val.k << ")";
                return ss.str();
            }
            case ValueKind::String: return str_val;
            case ValueKind::Tensor: {
                std::ostringstream ss;
                ss << "[";
                for (size_t i = 0; i < tensor_val.data.size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << tensor_val.data[i];
                }
                ss << "]";
                return ss.str();
            }
        }
        return "unknown";
    }
};

// --- Hiérarchie de Promotion Numérique : int -> real -> complex -> quaternion ---

inline ValueKind get_widest_numeric_type(ValueKind a, ValueKind b) {
    auto score = [](ValueKind k) -> int {
        if (k == ValueKind::Int) return 1;
        if (k == ValueKind::Real) return 2;
        if (k == ValueKind::Complex) return 3;
        if (k == ValueKind::Quaternion) return 4;
        return 0;
    };
    int sa = score(a);
    int sb = score(b);
    int m = std::max(sa, sb);
    if (m == 4) return ValueKind::Quaternion;
    if (m == 3) return ValueKind::Complex;
    if (m == 2) return ValueKind::Real;
    if (m == 1) return ValueKind::Int;
    throw std::runtime_error("Types non numériques pour opération arithmétique");
}

inline Quaternion to_quaternion(const Value& v) {
    if (v.kind == ValueKind::Int) return Quaternion(static_cast<double>(v.int_val), 0, 0, 0);
    if (v.kind == ValueKind::Real) return Quaternion(v.real_val, 0, 0, 0);
    if (v.kind == ValueKind::Complex) return Quaternion(v.complex_val.real(), v.complex_val.imag(), 0, 0);
    if (v.kind == ValueKind::Quaternion) return v.quat_val;
    throw std::runtime_error("Conversion vers quaternion invalide");
}

inline std::complex<double> to_complex(const Value& v) {
    if (v.kind == ValueKind::Int) return {static_cast<double>(v.int_val), 0.0};
    if (v.kind == ValueKind::Real) return {v.real_val, 0.0};
    if (v.kind == ValueKind::Complex) return v.complex_val;
    throw std::runtime_error("Conversion vers complex invalide");
}

inline double to_real(const Value& v) {
    if (v.kind == ValueKind::Int) return static_cast<double>(v.int_val);
    if (v.kind == ValueKind::Real) return v.real_val;
    throw std::runtime_error("Conversion vers real invalide");
}

} // namespace onyx
