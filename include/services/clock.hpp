#pragma once
#include <cstdint>

namespace ecu {

class IClock {
public:
    virtual ~IClock() = default;
    virtual std::uint32_t nowMs() const = 0;
};

// Horloge de simulation déterministe : le temps n'avance que si on l'ordonne.
class SimClock : public IClock {
public:
    std::uint32_t nowMs() const override { return nowMs_; }
    void advance(std::uint32_t ms) { nowMs_ += ms; }

private:
    std::uint32_t nowMs_ = 0;
};

}  // namespace ecu
