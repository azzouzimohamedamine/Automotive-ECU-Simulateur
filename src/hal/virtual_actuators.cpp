#include "hal/virtual_actuators.hpp"
#include <iostream>

namespace ecu {

void VirtualActuatorHal::apply(const ActuatorCommand& command) {
    last_ = command;
    ++count_;
    if (verbose_) {
        std::cout << "[ACTUATOR] lamp=" << (command.warningLamp ? "ON" : "OFF")
                  << " torque=" << command.requestedTorquePercent << "%\n";
    }
}

}  // namespace ecu
