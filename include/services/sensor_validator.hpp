#pragma once
#include "common/types.hpp"
#include "hal/i_sensor_hal.hpp"

namespace ecu {

struct Limits {
    double min;
    double max;
};

Limits limitsFor(SensorId id);

// Fonction pure : Missing > Invalid (NaN/inf) > OutOfRange > Valid.
SensorStatus validate(SensorId id, const RawReading& reading);

}  // namespace ecu
