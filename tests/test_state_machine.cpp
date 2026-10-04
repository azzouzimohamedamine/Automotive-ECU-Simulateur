#include <limits>
#include "minitest.hpp"
#include "test_rig.hpp"

using namespace ecu;
static const double kNaN = std::numeric_limits<double>::quiet_NaN();

TEST(sm_starts_in_init_and_init_success_goes_normal) {
    Rig r;
    CHECK_EQ(r.ecu.state(), EcuState::Init);
    r.run(1);
    CHECK_EQ(r.ecu.state(), EcuState::Normal);
    CHECK_EQ(r.ecu.transitions().size(), std::size_t{1});
    CHECK_EQ(r.ecu.transitions()[0].from, EcuState::Init);
    CHECK_EQ(r.ecu.transitions()[0].to, EcuState::Normal);
}

TEST(sm_init_failure_goes_error_with_d006) {
    Rig r;
    r.hal.setReady(false);
    r.run(1);
    CHECK_EQ(r.ecu.state(), EcuState::Error);
    CHECK(r.diag.isActive(FaultCode::D006_InitFailed));
    CHECK(r.act.last().warningLamp);
    CHECK_NEAR(r.act.last().requestedTorquePercent, 0.0, 1e-9);
}

TEST(sm_nominal_scenario_stays_normal_without_faults) {
    Rig r;
    r.run(100);
    CHECK_EQ(r.ecu.state(), EcuState::Normal);
    CHECK_EQ(r.diag.activeCount(), std::size_t{0});
    CHECK_EQ(r.diag.log().size(), std::size_t{0});
}

TEST(sm_overheat_goes_degraded_after_debounce) {
    Rig r;
    r.run(1);
    r.set(SensorId::EngineTemperature, 110.0);
    r.run(2);
    CHECK_EQ(r.ecu.state(), EcuState::Normal);  // anti-rebond : pas encore confirmé
    r.run(1);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    CHECK(r.diag.isActive(FaultCode::D007_EngineOverheat));
}

TEST(sm_temperature_out_of_range_goes_degraded) {
    Rig r;
    r.run(1);
    r.set(SensorId::EngineTemperature, 130.0);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    CHECK(r.diag.isActive(FaultCode::D001_TempOutOfRange));
}

TEST(sm_speed_out_of_range_goes_degraded) {
    Rig r;
    r.run(1);
    r.set(SensorId::VehicleSpeed, 250.0);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    CHECK(r.diag.isActive(FaultCode::D003_SpeedOutOfRange));
}

TEST(sm_nan_sensor_goes_degraded_via_d004) {
    Rig r;
    r.run(1);
    r.set(SensorId::EngineTemperature, kNaN);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    CHECK(r.diag.isActive(FaultCode::D004_SensorInvalid));
}

TEST(sm_degraded_recovers_only_after_stability_time) {
    Rig r;
    r.run(1);
    r.set(SensorId::EngineTemperature, 110.0);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    r.set(SensorId::EngineTemperature, 90.0);
    r.run(3 + 5);  // défaut guéri + 500 ms de stabilité
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    r.run(10);
    CHECK_EQ(r.ecu.state(), EcuState::Normal);
    CHECK(!r.act.last().warningLamp);
}

TEST(sm_recovery_timer_restarts_if_fault_reappears) {
    Rig r;
    r.run(1);
    r.set(SensorId::EngineTemperature, 110.0);
    r.run(3);
    r.set(SensorId::EngineTemperature, 90.0);
    r.run(3 + 5);
    r.set(SensorId::EngineTemperature, 110.0);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    r.set(SensorId::EngineTemperature, 90.0);
    r.run(3 + 5);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);  // le chrono a bien redémarré
}

TEST(sm_short_sensor_dropout_is_filtered) {
    Rig r;
    r.run(1);
    r.hal.setMissing(SensorId::OilPressure, true);
    r.run(2);
    r.hal.setMissing(SensorId::OilPressure, false);
    r.run(10);
    CHECK_EQ(r.ecu.state(), EcuState::Normal);
    CHECK_EQ(r.diag.log().size(), std::size_t{0});
}

TEST(sm_medium_dropout_degrades_then_recovers) {
    Rig r;
    r.run(1);
    r.hal.setMissing(SensorId::VehicleSpeed, true);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    r.hal.setMissing(SensorId::VehicleSpeed, false);
    r.run(20);
    CHECK_EQ(r.ecu.state(), EcuState::Normal);
}

TEST(sm_long_sensor_timeout_goes_error) {
    Rig r;
    r.run(1);
    r.hal.setMissing(SensorId::OilPressure, true);
    r.run(4);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);   // 300 ms : D004 seulement
    r.run(4);                                      // > 500 ms : timeout
    CHECK_EQ(r.ecu.state(), EcuState::Error);
    CHECK(r.diag.isActive(FaultCode::D005_SensorTimeout));
}

TEST(sm_oil_pressure_out_of_range_is_critical) {
    Rig r;
    r.run(1);
    r.set(SensorId::OilPressure, 12.0);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Error);
    CHECK(r.diag.isActive(FaultCode::D002_OilPressureOutOfRange));
}

TEST(sm_low_oil_pressure_plausibility_depends_on_speed) {
    Rig moving;
    moving.run(1);
    moving.set(SensorId::OilPressure, 0.5);
    moving.run(3);
    CHECK_EQ(moving.ecu.state(), EcuState::Error);
    CHECK(moving.diag.isActive(FaultCode::D008_LowOilPressure));

    Rig stopped;
    stopped.set(SensorId::VehicleSpeed, 0.0);
    stopped.run(1);
    stopped.set(SensorId::OilPressure, 0.5);
    stopped.run(10);
    CHECK_EQ(stopped.ecu.state(), EcuState::Normal);
}

TEST(sm_critical_fault_from_degraded_goes_error) {
    Rig r;
    r.run(1);
    r.set(SensorId::EngineTemperature, 110.0);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Degraded);
    r.set(SensorId::OilPressure, 12.0);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Error);
}

TEST(sm_error_never_recovers_automatically) {
    Rig r;
    r.run(1);
    r.set(SensorId::OilPressure, 12.0);
    r.run(3);
    CHECK_EQ(r.ecu.state(), EcuState::Error);
    r.set(SensorId::OilPressure, 3.0);   // la mesure redevient normale...
    r.run(100);
    CHECK_EQ(r.ecu.state(), EcuState::Error);  // ...mais l'erreur reste verrouillée
    CHECK(r.diag.isActive(FaultCode::D002_OilPressureOutOfRange));
}

TEST(sm_reset_refused_when_vehicle_moving) {
    Rig r;
    r.run(1);
    r.set(SensorId::OilPressure, 12.0);
    r.run(3);
    r.set(SensorId::OilPressure, 3.0);
    r.run(1);
    CHECK(!r.ecu.requestReset());
    CHECK_EQ(r.ecu.state(), EcuState::Error);
}

TEST(sm_reset_refused_when_sensor_not_valid) {
    Rig r;
    r.run(1);
    r.set(SensorId::OilPressure, 12.0);
    r.run(3);
    r.set(SensorId::VehicleSpeed, 0.0);
    r.run(1);  // pression encore hors plage
    CHECK(!r.ecu.requestReset());
    CHECK_EQ(r.ecu.state(), EcuState::Error);
}

TEST(sm_reset_accepted_when_safe_then_init_to_normal) {
    Rig r;
    r.run(1);
    r.set(SensorId::OilPressure, 12.0);
    r.run(3);
    r.set(SensorId::OilPressure, 3.0);
    r.set(SensorId::VehicleSpeed, 0.0);
    r.run(1);
    CHECK(r.ecu.requestReset());
    CHECK_EQ(r.ecu.state(), EcuState::Init);
    CHECK_EQ(r.diag.activeCount(), std::size_t{0});
    r.run(1);
    CHECK_EQ(r.ecu.state(), EcuState::Normal);
}

TEST(sm_reset_ignored_outside_error_state) {
    Rig r;
    CHECK(!r.ecu.requestReset());  // INIT
    r.run(1);
    CHECK(!r.ecu.requestReset());  // NORMAL
    CHECK_EQ(r.ecu.state(), EcuState::Normal);
}

TEST(sm_commands_per_state) {
    const ActuatorCommand i = EcuController::commandFor(EcuState::Init);
    const ActuatorCommand n = EcuController::commandFor(EcuState::Normal);
    const ActuatorCommand d = EcuController::commandFor(EcuState::Degraded);
    const ActuatorCommand e = EcuController::commandFor(EcuState::Error);
    CHECK(!i.warningLamp); CHECK_NEAR(i.requestedTorquePercent, 0.0, 1e-9);
    CHECK(!n.warningLamp); CHECK_NEAR(n.requestedTorquePercent, 50.0, 1e-9);
    CHECK(d.warningLamp);  CHECK_NEAR(d.requestedTorquePercent, 20.0, 1e-9);
    CHECK(e.warningLamp);  CHECK_NEAR(e.requestedTorquePercent, 0.0, 1e-9);
}

TEST(sm_actuators_follow_state_every_cycle) {
    Rig r;
    r.run(5);
    CHECK_EQ(r.act.applyCount(), std::size_t{5});
    CHECK_NEAR(r.act.last().requestedTorquePercent, 50.0, 1e-9);
    r.set(SensorId::EngineTemperature, 110.0);
    r.run(3);
    CHECK(r.act.last().warningLamp);
    CHECK_NEAR(r.act.last().requestedTorquePercent, 20.0, 1e-9);
}

TEST(sm_transition_log_records_time_and_reason) {
    Rig r;
    r.run(1);
    r.set(SensorId::EngineTemperature, 110.0);
    r.run(3);
    const auto& t = r.ecu.transitions();
    CHECK_EQ(t.size(), std::size_t{2});
    CHECK_EQ(t[1].from, EcuState::Normal);
    CHECK_EQ(t[1].to, EcuState::Degraded);
    CHECK_EQ(t[1].timeMs, 300u);
    CHECK(std::string(t[1].reason) == "recoverable fault");
}
