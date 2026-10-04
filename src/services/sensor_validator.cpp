#include "services/sensor_validator.hpp"
#include <cmath>
#include "common/config.hpp"

namespace ecu {

Limits limitsFor(SensorId id) {
    switch (id) {
        case SensorId::EngineTemperature: return {config::kTempMin, config::kTempMax};
        case SensorId::VehicleSpeed:      return {config::kSpeedMin, config::kSpeedMax};
        case SensorId::OilPressure:       return {config::kOilMin, config::kOilMax};
    }
    return {0.0, 0.0};
}

SensorStatus validate(SensorId id, const RawReading& reading) {
    if (!reading.available) return SensorStatus::Missing;
    if (!std::isfinite(reading.value)) return SensorStatus::Invalid;
    const Limits lim = limitsFor(id);
    if (reading.value < lim.min || reading.value > lim.max) return SensorStatus::OutOfRange;
    return SensorStatus::Valid;
}

}  // namespace ecu
