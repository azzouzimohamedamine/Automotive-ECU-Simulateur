#pragma once
#include "application/ecu_controller.hpp"
#include "common/config.hpp"
#include "hal/virtual_actuators.hpp"
#include "hal/virtual_sensors.hpp"
#include "services/clock.hpp"
#include "services/diagnostic.hpp"
#include "services/sensor_service.hpp"

// Banc de test : assemble HAL simulée + services + application, temps déterministe.
struct Rig {
    ecu::SimClock clock;
    ecu::VirtualSensorHal hal;
    ecu::VirtualActuatorHal act;
    ecu::SensorService sensors{hal, clock};
    ecu::DiagnosticService diag{clock};
    ecu::EcuController ecu{sensors, diag, act, clock};

    void run(unsigned cycles) {
        for (unsigned i = 0; i < cycles; ++i) {
            ecu.step();
            clock.advance(ecu::config::kCyclePeriodMs);
        }
    }
    void set(ecu::SensorId id, double v) { hal.setValue(id, v); }
};
