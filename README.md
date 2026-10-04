# Automotive ECU Simulator (C++17)

ECU de supervision automobile **entièrement simulé** : machine à états, capteurs/actionneurs
virtuels, gestion des défauts, temporisations, tests unitaires et couverture.

> Architecture pédagogique **inspirée d'AUTOSAR**, pas une implémentation certifiée de la norme.
> Seuils, codes défaut et commandes sont fictifs et ne modélisent pas la sécurité d'un véhicule réel.

## Fonctionnalités
- Machine à états `INIT → NORMAL ⇄ DEGRADED → ERROR` (reset explicite et sécurisé depuis ERROR)
- 3 couches : application / services / abstraction matérielle (interfaces + injection de dépendances)
- Capteurs virtuels avec injection de défauts : hors plage, NaN, capteur absent, HAL non prête
- Validation des mesures, contrôles de plausibilité, anti-rebond des défauts, journal horodaté
- Temporisations : timeout capteur (500 ms), temps de stabilité avant récupération (1000 ms)
- Horloge de simulation déterministe (+ mode temps réel `--realtime`)
- 39 tests unitaires, couverture gcov/gcovr, CI GitHub Actions

## Compiler et exécuter
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/ecu_sim              # scénario de démonstration (ou --realtime)
ctest --test-dir build --output-on-failure
```

## Couverture
```bash
pip install gcovr
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DECU_COVERAGE=ON
cmake --build build-cov --target coverage    # rapport HTML : build-cov/coverage.html
```
Résultat de référence : lignes 97,5 %, fonctions 100 %, branches 85,7 %.

## Exemple d'exécution
```
[t=1.0s] EVENT Injection : temperature 110 C (surchauffe)
[t=1.2s] DTC D007 ACTIF  Surchauffe moteur (RECOVERABLE)
[t=1.2s] STATE NORMAL -> DEGRADED  (recoverable fault)  | lamp=ON torque=20.0%
[t=4.2s] STATE DEGRADED -> NORMAL  (recovered)          | lamp=OFF torque=50.0%
[t=10.5s] DTC D005 ACTIF  Timeout capteur (CRITICAL)
[t=10.5s] STATE DEGRADED -> ERROR  (critical fault)
[t=11.5s]     -> reset REFUSE   (vehicule en mouvement)
[t=12.0s]     -> reset ACCEPTE  (a l'arret)  => INIT -> NORMAL
```

## Documentation
- [Architecture, UML, diagramme de séquence, correspondance AUTOSAR](docs/architecture.md)
- [Machine à états et table des défauts](docs/state_machine.md)
- [Plan de test et résultats](docs/test_plan.md)
- [Étapes de réalisation détaillées](docs/ETAPES.md)

## Limites
Simulation mono-thread sans OS temps réel ; pas de CAN/UDS ; pas de configuration ARXML ni RTE ;
pas de preuve de sûreté fonctionnelle (ISO 26262). Pistes : stack CAN simulée, NvM simulé,
port sur STM32/ESP32 en remplaçant uniquement la couche HAL.
