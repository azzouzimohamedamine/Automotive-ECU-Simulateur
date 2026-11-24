# Plan de test et résultats

Exécution : 

Résultat de référence : **41 / 41 tests réussis**.

## Scénarios de validation de haut niveau

| ID | Scénario | Résultat attendu | Test |
|---|---|---|---|
| TC-01 | Démarrage normal | INIT → NORMAL | `sm_starts_in_init_and_init_success_goes_normal` |
| TC-02 | Échec d'initialisation | ERROR + D006 | `sm_init_failure_goes_error_with_d006` |
| TC-03 | Fonctionnement nominal 10 s | NORMAL, aucun défaut | `sm_nominal_scenario_stays_normal_without_faults` |
| TC-04 | Surchauffe > 105 °C | DEGRADED après 3 cycles (pas avant) | `sm_overheat_goes_degraded_after_debounce` |
| TC-05 | Température / vitesse hors plage | DEGRADED | `sm_temperature_out_of_range_...`, `sm_speed_out_of_range_...` |
| TC-06 | Valeur NaN | DEGRADED via D004 | `sm_nan_sensor_goes_degraded_via_d004` |
| TC-07 | Retour à la normale | NORMAL seulement après 1000 ms de stabilité | `sm_degraded_recovers_only_after_stability_time` |
| TC-08 | Défaut qui réapparaît | Chronomètre de récupération relancé | `sm_recovery_timer_restarts_if_fault_reappears` |
| TC-09 | Micro-coupure capteur (2 cycles) | Filtrée, reste NORMAL | `sm_short_sensor_dropout_is_filtered` |
| TC-10 | Capteur absent 300 ms puis rétabli | DEGRADED puis NORMAL | `sm_medium_dropout_degrades_then_recovers` |
| TC-11 | Capteur absent ≥ 500 ms | ERROR + D005 | `sm_long_sensor_timeout_goes_error` |
| TC-12 | Pression d'huile hors plage | ERROR + D002 | `sm_oil_pressure_out_of_range_is_critical` |
| TC-13 | Plausibilité pression basse | ERROR si véhicule roule, NORMAL à l'arrêt | `sm_low_oil_pressure_plausibility_depends_on_speed` |
| TC-14 | Critique depuis DEGRADED | ERROR | `sm_critical_fault_from_degraded_goes_error` |
| TC-15 | Récupération automatique interdite | Reste ERROR 10 s après retour à la normale | `sm_error_never_recovers_automatically` |
| TC-16 | Reset refusé (véhicule en mouvement / capteur invalide) | Reste ERROR | `sm_reset_refused_when_...` (3 variantes : mouvement, capteur invalide, vitesse inconnue) |
| TC-17 | Reset accepté à l'arrêt | ERROR → INIT → NORMAL, défauts effacés | `sm_reset_accepted_when_safe_then_init_to_normal` |
| TC-18 | Reset hors ERROR | Ignoré | `sm_reset_ignored_outside_error_state` |
| TC-19 | Commandes actionneurs par état | Table `state_machine.md` | `sm_commands_per_state`, `sm_actuators_follow_state_every_cycle` |
| TC-20 | Journal des transitions | from/to/horodatage/raison | `sm_transition_log_records_time_and_reason` |


