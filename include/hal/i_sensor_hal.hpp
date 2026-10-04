#pragma once
#include "common/types.hpp"

namespace ecu {

struct RawReading {
    double value = 0.0;
    bool available = false;  // false = capteur muet / trame absente
};

// Abstraction matérielle : l'application ne connaît que cette interface.
class ISensorHal {
public:
    virtual ~ISensorHal() = default;
    virtual bool isReady() const = 0;
    virtual RawReading read(SensorId id) = 0;
};

}  // namespace ecu
