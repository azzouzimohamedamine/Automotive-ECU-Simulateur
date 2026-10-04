#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "common/types.hpp"
#include "services/clock.hpp"

namespace ecu {

// Codes propres au projet : PAS des codes défaut constructeur normalisés.
enum class FaultCode : std::uint8_t {
    D001_TempOutOfRange,
    D002_OilPressureOutOfRange,
    D003_SpeedOutOfRange,
    D004_SensorInvalid,
    D005_SensorTimeout,
    D006_InitFailed,
    D007_EngineOverheat,
    D008_LowOilPressure
};
constexpr std::size_t kFaultCount = 8;

struct FaultInfo {
    const char* id;
    const char* description;
    Severity severity;
    std::uint16_t failThreshold;  // cycles consécutifs en échec pour confirmer
    std::uint16_t passThreshold;  // cycles consécutifs OK pour guérir
};

struct DiagEvent {
    std::uint32_t timeMs;
    FaultCode code;
    bool active;  // true = défaut confirmé, false = défaut guéri
};

// Gestion des défauts façon "Dem" simplifié : anti-rebond, gravité, journal horodaté.
class DiagnosticService {
public:
    explicit DiagnosticService(const IClock& clock) : clock_(clock) {}

    void report(FaultCode code, bool failed);
    bool isActive(FaultCode code) const;
    bool anyActive(Severity severity) const;
    std::size_t activeCount() const;
    void clearAll();

    const std::vector<DiagEvent>& log() const { return log_; }
    static const FaultInfo& info(FaultCode code);

private:
    struct Entry {
        std::uint16_t failCount = 0;
        std::uint16_t passCount = 0;
        bool active = false;
    };

    const IClock& clock_;
    std::array<Entry, kFaultCount> entries_{};
    std::vector<DiagEvent> log_;
};

}  // namespace ecu
