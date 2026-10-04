#include <limits>
#include "minitest.hpp"
#include "hal/virtual_actuators.hpp"
#include "hal/virtual_sensors.hpp"
#include "services/sensor_service.hpp"

using namespace ecu;

static constexpr std::size_t P = static_cast<std::size_t>(SensorId::OilPressure);

TEST(sensor_service_nominal_snapshot) {
    SimClock clk;
    VirtualSensorHal hal;
    SensorService s(hal, clk);
    const Snapshot snap = s.update();
    for (const auto& x : snap) CHECK_EQ(x.status, SensorStatus::Valid);
    CHECK_NEAR(snap[static_cast<std::size_t>(SensorId::EngineTemperature)].value, 90.0, 1e-9);
}

TEST(sensor_service_tracks_missing_duration_and_reset) {
    SimClock clk;
    VirtualSensorHal hal;
    SensorService s(hal, clk);
    hal.setMissing(SensorId::OilPressure, true);
    CHECK_EQ(s.update()[P].missingForMs, 0u);
    clk.advance(100);
    CHECK_EQ(s.update()[P].missingForMs, 100u);
    clk.advance(400);
    const Snapshot snap = s.update();
    CHECK_EQ(snap[P].missingForMs, 500u);
    CHECK_EQ(snap[P].status, SensorStatus::Missing);
    hal.setMissing(SensorId::OilPressure, false);
    clk.advance(100);
    CHECK_EQ(s.update()[P].missingForMs, 0u);
    hal.setMissing(SensorId::OilPressure, true);  // nouvelle absence : repart de zéro
    clk.advance(100);
    CHECK_EQ(s.update()[P].missingForMs, 0u);
}

TEST(sensor_service_invalid_is_not_counted_as_missing) {
    SimClock clk;
    VirtualSensorHal hal;
    SensorService s(hal, clk);
    hal.setValue(SensorId::OilPressure, std::numeric_limits<double>::quiet_NaN());
    s.update();
    clk.advance(1000);
    const Snapshot snap = s.update();
    CHECK_EQ(snap[P].status, SensorStatus::Invalid);
    CHECK_EQ(snap[P].missingForMs, 0u);
}

TEST(sensor_service_self_test_follows_hal_ready_flag) {
    SimClock clk;
    VirtualSensorHal hal;
    SensorService s(hal, clk);
    CHECK(s.selfTest());
    hal.setReady(false);
    CHECK(!s.selfTest());
}

TEST(virtual_actuator_records_last_command) {
    VirtualActuatorHal a;
    CHECK_EQ(a.applyCount(), std::size_t{0});
    a.apply(ActuatorCommand{true, 20.0});
    CHECK(a.last().warningLamp);
    CHECK_NEAR(a.last().requestedTorquePercent, 20.0, 1e-9);
    CHECK_EQ(a.applyCount(), std::size_t{1});
    a.setVerbose(true);  // chemin d'affichage
    a.apply(ActuatorCommand{false, 50.0});
    CHECK_EQ(a.applyCount(), std::size_t{2});
    CHECK(!a.last().warningLamp);
}
