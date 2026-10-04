#include "minitest.hpp"
#include "services/diagnostic.hpp"

using namespace ecu;

TEST(diag_fault_confirmed_only_after_debounce) {
    SimClock clk;
    DiagnosticService d(clk);
    d.report(FaultCode::D001_TempOutOfRange, true);
    d.report(FaultCode::D001_TempOutOfRange, true);
    CHECK(!d.isActive(FaultCode::D001_TempOutOfRange));
    d.report(FaultCode::D001_TempOutOfRange, true);
    CHECK(d.isActive(FaultCode::D001_TempOutOfRange));
    CHECK_EQ(d.activeCount(), std::size_t{1});
}

TEST(diag_intermittent_failure_does_not_confirm) {
    SimClock clk;
    DiagnosticService d(clk);
    d.report(FaultCode::D001_TempOutOfRange, true);
    d.report(FaultCode::D001_TempOutOfRange, true);
    d.report(FaultCode::D001_TempOutOfRange, false);  // compteur remis à zéro
    d.report(FaultCode::D001_TempOutOfRange, true);
    d.report(FaultCode::D001_TempOutOfRange, true);
    CHECK(!d.isActive(FaultCode::D001_TempOutOfRange));
}

TEST(diag_fault_heals_after_pass_threshold) {
    SimClock clk;
    DiagnosticService d(clk);
    for (int i = 0; i < 3; ++i) d.report(FaultCode::D003_SpeedOutOfRange, true);
    CHECK(d.isActive(FaultCode::D003_SpeedOutOfRange));
    d.report(FaultCode::D003_SpeedOutOfRange, false);
    d.report(FaultCode::D003_SpeedOutOfRange, false);
    CHECK(d.isActive(FaultCode::D003_SpeedOutOfRange));
    d.report(FaultCode::D003_SpeedOutOfRange, false);
    CHECK(!d.isActive(FaultCode::D003_SpeedOutOfRange));
}

TEST(diag_immediate_faults_and_severity) {
    SimClock clk;
    DiagnosticService d(clk);
    CHECK(!d.anyActive(Severity::Critical));
    d.report(FaultCode::D006_InitFailed, true);  // seuil = 1
    CHECK(d.isActive(FaultCode::D006_InitFailed));
    CHECK(d.anyActive(Severity::Critical));
    CHECK(!d.anyActive(Severity::Recoverable));
    CHECK_EQ(DiagnosticService::info(FaultCode::D002_OilPressureOutOfRange).severity,
             Severity::Critical);
    CHECK_EQ(DiagnosticService::info(FaultCode::D001_TempOutOfRange).severity,
             Severity::Recoverable);
}

TEST(diag_log_is_timestamped_and_clear_resets) {
    SimClock clk;
    DiagnosticService d(clk);
    clk.advance(250);
    d.report(FaultCode::D006_InitFailed, true);
    CHECK_EQ(d.log().size(), std::size_t{1});
    CHECK_EQ(d.log()[0].timeMs, 250u);
    CHECK(d.log()[0].active);
    CHECK(std::string(DiagnosticService::info(d.log()[0].code).id) == "D006");
    d.clearAll();
    CHECK(!d.isActive(FaultCode::D006_InitFailed));
    CHECK_EQ(d.activeCount(), std::size_t{0});
}
