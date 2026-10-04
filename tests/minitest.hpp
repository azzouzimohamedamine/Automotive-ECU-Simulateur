#pragma once
// Mini framework de tests sans dépendance externe (remplaçable par GoogleTest).
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace minitest {

struct Case {
    const char* name;
    void (*fn)();
};

inline std::vector<Case>& registry() { static std::vector<Case> r; return r; }
inline int& failures() { static int f = 0; return f; }

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registry().push_back({name, fn}); }
};

template <class T>
std::string show(const T& v) {
    if constexpr (std::is_enum_v<T>) {
        return std::to_string(static_cast<long long>(v));
    } else {
        std::ostringstream o;
        o << v;
        return o.str();
    }
}

}  // namespace minitest

#define TEST(name)                                                         \
    static void name();                                                    \
    static minitest::Registrar registrar_##name(#name, &name);             \
    static void name()

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            ++minitest::failures();                                        \
            std::cerr << __FILE__ << ":" << __LINE__                       \
                      << ": CHECK failed: " #cond "\n";                    \
        }                                                                  \
    } while (0)

#define CHECK_EQ(a, b)                                                     \
    do {                                                                   \
        const auto va = (a);                                               \
        const auto vb = (b);                                               \
        if (!(va == vb)) {                                                 \
            ++minitest::failures();                                        \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK_EQ failed: " \
                      #a " == " #b " (" << minitest::show(va) << " vs "    \
                      << minitest::show(vb) << ")\n";                      \
        }                                                                  \
    } while (0)

#define CHECK_NEAR(a, b, eps) CHECK(std::fabs((a) - (b)) <= (eps))
