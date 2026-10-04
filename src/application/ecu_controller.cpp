#include "application/ecu_controller.hpp"
#include <algorithm>
#include "common/config.hpp"

namespace ecu {

namespace {
constexpr std::size_t T = static_cast<std::size_t>(SensorId::EngineTemperature);
constexpr std::size_t V = static_cast<std::size_t>(SensorId::VehicleSpeed);
constexpr std::size_t P = static_cast<std::size_t>(SensorId::OilPressure);
}  // namespace

EcuController::EcuController(SensorService& sensors, DiagnosticService& diag,
                             IActuatorHal& actuators, const IClock& clock)
    : sensors_(sensors), diag_(diag), actuators_(actuators), clock_(clock) {}

ActuatorCommand EcuController::commandFor(EcuState state) {
    ActuatorCommand c;
    switch (state) {
        case EcuState::Init:     c.warningLamp = false; c.requestedTorquePercent = 0.0;  break;
        case EcuState::Normal:   c.warningLamp = false; c.requestedTorquePercent = 50.0; break;
        case EcuState::Degraded: c.warningLamp = true;  c.requestedTorquePercent = 20.0; break;
        case EcuState::Error:    c.warningLamp = true;  c.requestedTorquePercent = 0.0;  break;
    }
    return c;
}

void EcuController::enterState(EcuState next, const char* reason) {
    transitions_.push_back({clock_.nowMs(), state_, next, reason});
    state_ = next;
    healthySinceValid_ = false;
}

void EcuController::step() {
    snapshot_ = sensors_.update();
    switch (state_) {
        case EcuState::Init:
            runInit();
            break;
        case EcuState::Normal:
        case EcuState::Degraded:
            evaluateFaults();
            supervise();
            break;
        case EcuState::Error:
            break;  // verrouillé : seule requestReset() permet d'en sortir
    }
    actuators_.apply(commandFor(state_));
}

void EcuController::runInit() {
    if (sensors_.selfTest()) {
        enterState(EcuState::Normal, "init ok");
    } else {
        diag_.report(FaultCode::D006_InitFailed, true);
        enterState(EcuState::Error, "init failed");
    }
}

void EcuController::evaluateFaults() {
    const SensorSample& t = snapshot_[T];
    const SensorSample& v = snapshot_[V];
    const SensorSample& p = snapshot_[P];

    const bool anyInvalidOrMissing = std::any_of(snapshot_.begin(), snapshot_.end(),
        [](const SensorSample& s) {
            return s.status == SensorStatus::Invalid || s.status == SensorStatus::Missing;
        });
    const bool anyTimeout = std::any_of(snapshot_.begin(), snapshot_.end(),
        [](const SensorSample& s) {
            return s.status == SensorStatus::Missing &&
                   s.missingForMs >= config::kSensorTimeoutMs;
        });

    // Contrôles de plage / validité
    diag_.report(FaultCode::D001_TempOutOfRange,        t.status == SensorStatus::OutOfRange);
    diag_.report(FaultCode::D002_OilPressureOutOfRange, p.status == SensorStatus::OutOfRange);
    diag_.report(FaultCode::D003_SpeedOutOfRange,       v.status == SensorStatus::OutOfRange);
    diag_.report(FaultCode::D004_SensorInvalid,         anyInvalidOrMissing);
    diag_.report(FaultCode::D005_SensorTimeout,         anyTimeout);

    // Contrôles de seuil / plausibilité (uniquement sur des mesures valides)
    diag_.report(FaultCode::D007_EngineOverheat,
                 t.status == SensorStatus::Valid && t.value > config::kTempWarn);
    diag_.report(FaultCode::D008_LowOilPressure,
                 p.status == SensorStatus::Valid && v.status == SensorStatus::Valid &&
                 v.value > config::kSpeedEngineRunning && p.value < config::kOilLow);
}

void EcuController::supervise() {
    if (diag_.anyActive(Severity::Critical)) {
        enterState(EcuState::Error, "critical fault");
        return;
    }
    const bool recoverable = diag_.anyActive(Severity::Recoverable);
    const std::uint32_t now = clock_.nowMs();

    if (state_ == EcuState::Normal) {
        if (recoverable) enterState(EcuState::Degraded, "recoverable fault");
        return;
    }
    // DEGRADED : retour à NORMAL seulement après une période de stabilité
    if (recoverable) {
        healthySinceValid_ = false;
        return;
    }
    if (!healthySinceValid_) {
        healthySinceValid_ = true;
        healthySinceMs_ = now;
    }
    if (now - healthySinceMs_ >= config::kRecoveryTimeMs) {
        enterState(EcuState::Normal, "recovered");
    }
}

bool EcuController::requestReset() {
    if (state_ != EcuState::Error) return false;
    const bool standstill = snapshot_[V].status == SensorStatus::Valid &&
                            snapshot_[V].value <= config::kSpeedStandstill;
    const bool sensorsOk = std::all_of(snapshot_.begin(), snapshot_.end(),
        [](const SensorSample& s) { return s.status == SensorStatus::Valid; });
    if (!standstill || !sensorsOk) return false;
    diag_.clearAll();
    enterState(EcuState::Init, "explicit reset");
    return true;
}

}  // namespace ecu
