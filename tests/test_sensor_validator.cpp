#include <cmath>
#include <limits>
#include "minitest.hpp"
#include "services/sensor_validator.hpp"

using namespace ecu;

static SensorStatus v(SensorId id, double x) { return validate(id, RawReading{x, true}); }
static const double kNaN = std::numeric_limits<double>::quiet_NaN();
static const double kInf = std::numeric_limits<double>::infinity();

TEST(validator_temperature_nominal_and_limits) {
    CHECK_EQ(v(SensorId::EngineTemperature, 90.0),  SensorStatus::Valid);
    CHECK_EQ(v(SensorId::EngineTemperature, 0.0),   SensorStatus::Valid);
    CHECK_EQ(v(SensorId::EngineTemperature, 120.0), SensorStatus::Valid);
    CHECK_EQ(v(SensorId::EngineTemperature, 130.0), SensorStatus::OutOfRange);
    CHECK_EQ(v(SensorId::EngineTemperature, -0.1),  SensorStatus::OutOfRange);
    CHECK_EQ(v(SensorId::EngineTemperature, 120.1), SensorStatus::OutOfRange);
}

TEST(validator_speed_limits) {
    CHECK_EQ(v(SensorId::VehicleSpeed, 0.0),   SensorStatus::Valid);
    CHECK_EQ(v(SensorId::VehicleSpeed, 200.0), SensorStatus::Valid);
    CHECK_EQ(v(SensorId::VehicleSpeed, -5.0),  SensorStatus::OutOfRange);
    CHECK_EQ(v(SensorId::VehicleSpeed, 200.5), SensorStatus::OutOfRange);
}

TEST(validator_oil_pressure_limits) {
    CHECK_EQ(v(SensorId::OilPressure, 3.0),  SensorStatus::Valid);
    CHECK_EQ(v(SensorId::OilPressure, 10.0), SensorStatus::Valid);
    CHECK_EQ(v(SensorId::OilPressure, 10.1), SensorStatus::OutOfRange);
    CHECK_EQ(v(SensorId::OilPressure, -0.1), SensorStatus::OutOfRange);
}

TEST(validator_non_finite_values_are_invalid) {
    CHECK_EQ(v(SensorId::EngineTemperature, kNaN), SensorStatus::Invalid);
    CHECK_EQ(v(SensorId::VehicleSpeed, kInf),      SensorStatus::Invalid);
    CHECK_EQ(v(SensorId::OilPressure, -kInf),      SensorStatus::Invalid);
}

TEST(validator_unavailable_reading_is_missing) {
    CHECK_EQ(validate(SensorId::OilPressure, RawReading{3.0, false}), SensorStatus::Missing);
    CHECK_EQ(validate(SensorId::OilPressure, RawReading{kNaN, false}), SensorStatus::Missing);
}

TEST(types_to_string) {
    CHECK(std::string(toString(EcuState::Init)) == "INIT");
    CHECK(std::string(toString(EcuState::Normal)) == "NORMAL");
    CHECK(std::string(toString(EcuState::Degraded)) == "DEGRADED");
    CHECK(std::string(toString(EcuState::Error)) == "ERROR");
    CHECK(std::string(toString(SensorStatus::Valid)) == "VALID");
    CHECK(std::string(toString(SensorStatus::OutOfRange)) == "OUT_OF_RANGE");
    CHECK(std::string(toString(SensorStatus::Invalid)) == "INVALID_VALUE");
    CHECK(std::string(toString(SensorStatus::Missing)) == "MISSING");
    CHECK(std::string(toString(SensorId::EngineTemperature)) == "EngineTemperature");
    CHECK(std::string(toString(SensorId::VehicleSpeed)) == "VehicleSpeed");
    CHECK(std::string(toString(SensorId::OilPressure)) == "OilPressure");
    CHECK(std::string(toString(Severity::Recoverable)) == "RECOVERABLE");
    CHECK(std::string(toString(Severity::Critical)) == "CRITICAL");
}
