#pragma once
#include "common/types.hpp"

namespace ecu {

class IActuatorHal {
public:
    virtual ~IActuatorHal() = default;
    virtual void apply(const ActuatorCommand& command) = 0;
};

}  // namespace ecu
