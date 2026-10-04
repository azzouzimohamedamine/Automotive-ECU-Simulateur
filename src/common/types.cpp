#include "common/types.hpp"

namespace ecu {

const char* toString(SensorId id) {
    switch (id) {
        case SensorId::EngineTemperature: return "EngineTemperature";
        case SensorId::VehicleSpeed:      return "VehicleSpeed";
        case SensorId::OilPressure:       return "OilPressure";
    }
    return "Unknown";
}

const char* toString(SensorStatus status) {
    switch (status) {
        case SensorStatus::Valid:      return "VALID";
        case SensorStatus::OutOfRange: return "OUT_OF_RANGE";
        case SensorStatus::Invalid:    return "INVALID_VALUE";
        case SensorStatus::Missing:    return "MISSING";
    }
    return "Unknown";
}

const char* toString(EcuState state) {
    switch (state) {
        case EcuState::Init:     return "INIT";
        case EcuState::Normal:   return "NORMAL";
        case EcuState::Degraded: return "DEGRADED";
        case EcuState::Error:    return "ERROR";
    }
    return "Unknown";
}

const char* toString(Severity severity) {
    switch (severity) {
        case Severity::Recoverable: return "RECOVERABLE";
        case Severity::Critical:    return "CRITICAL";
    }
    return "Unknown";
}

}  // namespace ecu
