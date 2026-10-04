#pragma once
#include <array>
#include <cstdint>
#include "common/types.hpp"
#include "hal/i_sensor_hal.hpp"
#include "services/clock.hpp"

namespace ecu {

struct SensorSample {
    double value = 0.0;
    SensorStatus status = SensorStatus::Missing;
    std::uint32_t missingForMs = 0;  // durée d'absence continue (temporisation)
};

using Snapshot = std::array<SensorSample, kSensorCount>;

// Lit la HAL, valide chaque mesure et suit la durée d'absence de chaque capteur.
class SensorService {
public:
    SensorService(ISensorHal& hal, const IClock& clock);

    bool selfTest() const { return hal_.isReady(); }
    Snapshot update();

private:
    ISensorHal& hal_;
    const IClock& clock_;
    std::array<bool, kSensorCount> missing_{};
    std::array<std::uint32_t, kSensorCount> missingSinceMs_{};
};

}  // namespace ecu
