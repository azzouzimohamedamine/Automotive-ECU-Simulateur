#include "services/diagnostic.hpp"

namespace ecu {

namespace {
using S = Severity;
// Ordre = ordre de l'enum FaultCode.
constexpr std::array<FaultInfo, kFaultCount> kTable{{
    {"D001", "Temperature hors plage",          S::Recoverable, 3, 3},
    {"D002", "Pression d'huile hors plage",     S::Critical,    3, 3},
    {"D003", "Vitesse hors plage",              S::Recoverable, 3, 3},
    {"D004", "Donnee capteur invalide/absente", S::Recoverable, 3, 3},
    {"D005", "Timeout capteur",                 S::Critical,    1, 3},
    {"D006", "Initialisation echouee",          S::Critical,    1, 3},
    {"D007", "Surchauffe moteur",               S::Recoverable, 3, 3},
    {"D008", "Pression d'huile trop basse",     S::Critical,    3, 3},
}};
constexpr std::size_t idx(FaultCode c) { return static_cast<std::size_t>(c); }
}  // namespace

const FaultInfo& DiagnosticService::info(FaultCode code) { return kTable[idx(code)]; }

void DiagnosticService::report(FaultCode code, bool failed) {
    Entry& e = entries_[idx(code)];
    const FaultInfo& inf = info(code);
    if (failed) {
        e.passCount = 0;
        if (e.failCount < UINT16_MAX) ++e.failCount;
        if (!e.active && e.failCount >= inf.failThreshold) {
            e.active = true;
            log_.push_back({clock_.nowMs(), code, true});
        }
    } else {
        e.failCount = 0;
        if (e.active) {
            if (e.passCount < UINT16_MAX) ++e.passCount;
            if (e.passCount >= inf.passThreshold) {
                e.active = false;
                e.passCount = 0;
                log_.push_back({clock_.nowMs(), code, false});
            }
        }
    }
}

bool DiagnosticService::isActive(FaultCode code) const { return entries_[idx(code)].active; }

bool DiagnosticService::anyActive(Severity severity) const {
    for (std::size_t i = 0; i < kFaultCount; ++i) {
        if (entries_[i].active && kTable[i].severity == severity) return true;
    }
    return false;
}

std::size_t DiagnosticService::activeCount() const {
    std::size_t n = 0;
    for (const Entry& e : entries_) n += e.active ? 1 : 0;
    return n;
}

void DiagnosticService::clearAll() { entries_ = {}; }

}  // namespace ecu
