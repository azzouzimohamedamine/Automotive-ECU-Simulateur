#pragma once
#include <cstdint>
#include <vector>
#include "common/types.hpp"
#include "hal/i_actuator_hal.hpp"
#include "services/clock.hpp"
#include "services/diagnostic.hpp"
#include "services/sensor_service.hpp"

namespace ecu {

struct Transition {
    std::uint32_t timeMs;
    EcuState from;
    EcuState to;
    const char* reason;
};

// Couche application : machine à états + décisions. Ne parle qu'aux services
// et à l'interface d'actionneurs (jamais au matériel directement).
class EcuController {
public:
    EcuController(SensorService& sensors, DiagnosticService& diag,
                  IActuatorHal& actuators, const IClock& clock);

    void step();           // un cycle de la tâche périodique
    bool requestReset();   // ERROR -> INIT, seulement si les conditions sont sûres

    EcuState state() const { return state_; }
    const std::vector<Transition>& transitions() const { return transitions_; }
    const Snapshot& snapshot() const { return snapshot_; }

    static ActuatorCommand commandFor(EcuState state);

private:
    void runInit();
    void evaluateFaults();
    void supervise();
    void enterState(EcuState next, const char* reason);

    SensorService& sensors_;
    DiagnosticService& diag_;
    IActuatorHal& actuators_;
    const IClock& clock_;

    EcuState state_ = EcuState::Init;
    Snapshot snapshot_{};
    std::vector<Transition> transitions_;
    bool healthySinceValid_ = false;
    std::uint32_t healthySinceMs_ = 0;
};

}  // namespace ecu
