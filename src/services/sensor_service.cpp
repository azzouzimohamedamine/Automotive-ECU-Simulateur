#include "services/sensor_service.hpp"
#include "services/sensor_validator.hpp"

namespace ecu {

SensorService::SensorService(ISensorHal& hal, const IClock& clock)
    : hal_(hal), clock_(clock) {}

Snapshot SensorService::update() {
    Snapshot snap{};
    const std::uint32_t now = clock_.nowMs();
    for (std::size_t i = 0; i < kSensorCount; ++i) {
        const SensorId id = static_cast<SensorId>(i);
        const RawReading raw = hal_.read(id);
        SensorSample& s = snap[i];
        s.value = raw.value;
        s.status = validate(id, raw);
        if (s.status == SensorStatus::Missing) {
            if (!missing_[i]) {
                missing_[i] = true;
                missingSinceMs_[i] = now;
            }
            s.missingForMs = now - missingSinceMs_[i];
        } else {
            missing_[i] = false;
        }
    }
    return snap;
}

}  // namespace ecu
