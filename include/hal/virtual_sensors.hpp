#pragma once
#include <array>
#include "hal/i_sensor_hal.hpp"

namespace ecu {

// Capteurs simulés avec injection de défauts (valeur, absence, HAL non prêt).
class VirtualSensorHal : public ISensorHal {
public:
    VirtualSensorHal();

    bool isReady() const override { return ready_; }
    RawReading read(SensorId id) override;

    void setValue(SensorId id, double value);
    void setMissing(SensorId id, bool missing);
    void setReady(bool ready) { ready_ = ready; }

private:
    std::array<double, kSensorCount> values_{};
    std::array<bool, kSensorCount> missing_{};
    bool ready_ = true;
};

}  // namespace ecu
