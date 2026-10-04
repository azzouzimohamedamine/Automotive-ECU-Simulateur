#pragma once
#include <cstddef>
#include <cstdint>

namespace ecu {

enum class SensorId : std::uint8_t { EngineTemperature, VehicleSpeed, OilPressure };
constexpr std::size_t kSensorCount = 3;

enum class SensorStatus : std::uint8_t { Valid, OutOfRange, Invalid, Missing };
enum class EcuState : std::uint8_t { Init, Normal, Degraded, Error };
enum class Severity : std::uint8_t { Recoverable, Critical };

struct ActuatorCommand {
    bool warningLamp = false;
    double requestedTorquePercent = 0.0;
};

const char* toString(SensorId id);
const char* toString(SensorStatus status);
const char* toString(EcuState state);
const char* toString(Severity severity);

}  // namespace ecu
