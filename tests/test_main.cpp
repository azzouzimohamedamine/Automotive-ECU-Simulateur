#include "minitest.hpp"

int main() {
    int failedTests = 0;
    for (const auto& c : minitest::registry()) {
        const int before = minitest::failures();
        c.fn();
        const bool ok = minitest::failures() == before;
        std::cout << (ok ? "[ PASS ] " : "[ FAIL ] ") << c.name << '\n';
        if (!ok) ++failedTests;
    }
    std::cout << "\n" << (minitest::registry().size() - static_cast<std::size_t>(failedTests))
              << "/" << minitest::registry().size() << " tests passes\n";
    return failedTests == 0 ? 0 : 1;
}
