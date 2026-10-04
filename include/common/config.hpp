#pragma once
#include <cstdint>

// Hypothèses de simulation (pédagogiques) - pas des spécifications constructeur.
namespace ecu::config {

constexpr std::uint32_t kCyclePeriodMs   = 100;   // période de la tâche cyclique
constexpr std::uint32_t kSensorTimeoutMs = 500;   // capteur absent -> défaut critique
constexpr std::uint32_t kRecoveryTimeMs  = 1000;  // stabilité requise avant DEGRADED -> NORMAL

constexpr double kTempMin = 0.0,  kTempMax = 120.0, kTempWarn = 105.0;
constexpr double kSpeedMin = 0.0, kSpeedMax = 200.0;
constexpr double kOilMin = 0.0,   kOilMax = 10.0,   kOilLow = 1.0;

constexpr double kSpeedEngineRunning = 10.0;  // plausibilité : moteur supposé tournant
constexpr double kSpeedStandstill    = 1.0;   // condition de sûreté pour un reset

}  // namespace ecu::config
