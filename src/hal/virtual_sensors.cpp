#include "hal/virtual_sensors.hpp"

namespace ecu {

namespace {
constexpr std::size_t idx(SensorId id) { return static_cast<std::size_t>(id); }
}

VirtualSensorHal::VirtualSensorHal() {
    // Scénario nominal
    values_[idx(SensorId::EngineTemperature)] = 90.0;
    values_[idx(SensorId::VehicleSpeed)] = 60.0;
    values_[idx(SensorId::OilPressure)] = 3.0;
}

RawReading VirtualSensorHal::read(SensorId id) {
    return RawReading{values_[idx(id)], !missing_[idx(id)]};
}

void VirtualSensorHal::setValue(SensorId id, double value) { values_[idx(id)] = value; }
void VirtualSensorHal::setMissing(SensorId id, bool missing) { missing_[idx(id)] = missing; }

}  // namespace ecu
