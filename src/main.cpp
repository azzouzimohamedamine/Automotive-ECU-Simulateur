#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "application/ecu_controller.hpp"
#include "common/config.hpp"
#include "hal/virtual_actuators.hpp"
#include "hal/virtual_sensors.hpp"
#include "services/clock.hpp"
#include "services/diagnostic.hpp"
#include "services/sensor_service.hpp"

using namespace ecu;

struct ScenarioEvent {
    unsigned cycle;
    const char* label;
    std::function<void()> action;
};

static void printTime(std::uint32_t ms) {
    std::cout << "[t=" << std::fixed << std::setprecision(1) << ms / 1000.0 << "s] ";
}

int main(int argc, char** argv) {
    const bool realtime = argc > 1 && std::strcmp(argv[1], "--realtime") == 0;

    // Composition : HAL -> services -> application (injection de dépendances)
    SimClock clock;
    VirtualSensorHal sensorsHal;
    VirtualActuatorHal actuatorsHal;
    SensorService sensors(sensorsHal, clock);
    DiagnosticService diag(clock);
    EcuController ecuCtl(sensors, diag, actuatorsHal, clock);

    EcuController* ecu = &ecuCtl;
    const std::vector<ScenarioEvent> scenario{
        {10,  "Injection : temperature 110 C (surchauffe)",
              [&] { sensorsHal.setValue(SensorId::EngineTemperature, 110.0); }},
        {30,  "Retour temperature 90 C",
              [&] { sensorsHal.setValue(SensorId::EngineTemperature, 90.0); }},
        {60,  "Injection : vitesse 250 km/h (hors plage)",
              [&] { sensorsHal.setValue(SensorId::VehicleSpeed, 250.0); }},
        {70,  "Retour vitesse 60 km/h",
              [&] { sensorsHal.setValue(SensorId::VehicleSpeed, 60.0); }},
        {100, "Injection : capteur de pression d'huile muet",
              [&] { sensorsHal.setMissing(SensorId::OilPressure, true); }},
        {110, "Capteur de pression d'huile rétabli (ERROR doit rester verrouillé)",
              [&] { sensorsHal.setMissing(SensorId::OilPressure, false); }},
        {115, "Demande de reset a 60 km/h (doit etre refusee)",
              [&] { std::cout << "    -> reset " << (ecu->requestReset() ? "ACCEPTE" : "REFUSE") << '\n'; }},
        {118, "Vehicule a l'arret",
              [&] { sensorsHal.setValue(SensorId::VehicleSpeed, 0.0); }},
        {120, "Demande de reset a l'arret (doit etre acceptee)",
              [&] { std::cout << "    -> reset " << (ecu->requestReset() ? "ACCEPTE" : "REFUSE") << '\n'; }},
    };

    using SteadyClock = std::chrono::steady_clock;
    auto nextWake = SteadyClock::now();
    std::size_t shownTransitions = 0, shownDiag = 0;
    constexpr unsigned kCycles = 140;

    std::cout << "=== Simulateur ECU (architecture pedagogique inspiree d'AUTOSAR) ===\n";
    for (unsigned cycle = 0; cycle < kCycles; ++cycle) {
        for (const auto& ev : scenario) {
            if (ev.cycle == cycle) {
                printTime(clock.nowMs());
                std::cout << "EVENT " << ev.label << '\n';
                ev.action();
            }
        }

        ecuCtl.step();

        for (; shownDiag < diag.log().size(); ++shownDiag) {
            const DiagEvent& d = diag.log()[shownDiag];
            printTime(d.timeMs);
            const FaultInfo& inf = DiagnosticService::info(d.code);
            std::cout << "DTC " << inf.id << " " << (d.active ? "ACTIF  " : "GUERI  ")
                      << inf.description << " (" << toString(inf.severity) << ")\n";
        }
        for (; shownTransitions < ecuCtl.transitions().size(); ++shownTransitions) {
            const Transition& t = ecuCtl.transitions()[shownTransitions];
            printTime(t.timeMs);
            std::cout << "STATE " << toString(t.from) << " -> " << toString(t.to)
                      << "  (" << t.reason << ")  | lamp="
                      << (EcuController::commandFor(t.to).warningLamp ? "ON" : "OFF")
                      << " torque=" << EcuController::commandFor(t.to).requestedTorquePercent
                      << "%\n";
        }

        clock.advance(config::kCyclePeriodMs);
        if (realtime) {
            nextWake += std::chrono::milliseconds(config::kCyclePeriodMs);
            std::this_thread::sleep_until(nextWake);
        }
    }

    std::cout << "=== Fin : etat final " << toString(ecuCtl.state())
              << ", " << diag.log().size() << " evenements diagnostic, "
              << ecuCtl.transitions().size() << " transitions ===\n";
    return 0;
}
