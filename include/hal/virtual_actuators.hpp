#pragma once
#include <cstddef>
#include "hal/i_actuator_hal.hpp"

namespace ecu {

// Actionneurs simulés : mémorisent (et affichent si verbose) la dernière commande.
class VirtualActuatorHal : public IActuatorHal {
public:
    void apply(const ActuatorCommand& command) override;

    const ActuatorCommand& last() const { return last_; }
    std::size_t applyCount() const { return count_; }
    void setVerbose(bool verbose) { verbose_ = verbose; }

private:
    ActuatorCommand last_{};
    std::size_t count_ = 0;
    bool verbose_ = false;
};

}  // namespace ecu
