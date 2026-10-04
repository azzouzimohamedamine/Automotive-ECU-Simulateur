# Étapes de réalisation détaillées

Chaque étape = un objectif, des actions, un **critère de validation** et un commit.
Durée indicative : 5 semaines à ~6 h/semaine. Le dépôt contient la solution finale ;
refais les étapes dans l'ordre en t'en servant comme référence.

## Étape 0 — Outils (1 h)
- **Windows** : installer MSYS2, puis dans le terminal *UCRT64* :
  `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja git`
- **Linux** : `sudo apt install g++ cmake ninja-build git python3-pip`
- Puis `pip install gcovr`. VS Code + extensions *C/C++* et *CMake Tools*.
- ✅ Validation : `g++ --version`, `cmake --version`, `git --version` répondent.

## Étape 1 — Dépôt et arborescence (30 min)
- `git init`, créer `include/ src/ tests/ docs/`, `.gitignore`, `README.md` vide.
- ✅ `git status` propre après le premier commit : `chore: project skeleton`.

## Étape 2 — Exigences (1 h) → `docs/state_machine.md`
- Écrire la table des états, des transitions, des défauts (code, gravité, seuils, durées).
- Décider tôt : qui décide de la gravité (table de diagnostic), ERROR verrouillé, reset explicite.
- ✅ Chaque ligne de la table pourra devenir un test (TC-xx). Commit : `docs: requirements and state table`.

## Étape 3 — Prototype de machine à états (2 h)
- Un seul `main.cpp` avec `enum class EcuState` et des méthodes de transition (code de ton guide).
- ✅ Les 5 lignes d'affichage attendues apparaissent. Commit : `feat: state machine prototype`.

## Étape 4 — Types communs et couche HAL (3 h)
- `common/types.hpp` (enums, `ActuatorCommand`), `common/config.hpp` (seuils, durées).
- Interfaces `ISensorHal`, `IActuatorHal` ; implémentations `VirtualSensorHal` (setValue,
  setMissing, setReady) et `VirtualActuatorHal` (mémorise la dernière commande).
- ✅ Un petit `main` lit les 3 capteurs et affiche une commande. Commit : `feat: hardware abstraction layer`.

## Étape 5 — Services (6 h)
1. `IClock` + `SimClock` : temps déterministe (jamais `sleep` dans la logique).
2. `sensor_validator` : fonction **pure** `validate()` → Missing > Invalid > OutOfRange > Valid.
3. `SensorService` : lit la HAL, valide, mesure la durée d'absence de chaque capteur.
4. `DiagnosticService` : table de défauts, anti-rebond par compteurs (confirmation/guérison),
   gravité, journal horodaté, `clearAll()`.
- ✅ Chaque service se teste seul, sans l'application. Commits : un par service.

## Étape 6 — Application `EcuController` (6 h)
- `step()` : `sensors.update()` → (INIT : auto-test) ou (NORMAL/DEGRADED : `evaluateFaults()` puis
  `supervise()`) → `actuators.apply(commandFor(state))`.
- `evaluateFaults()` : appelle `diag.report(code, condition)` pour chaque défaut.
- `supervise()` : critique → ERROR ; récupérable → DEGRADED ; retour NORMAL après `kRecoveryTimeMs`.
- `requestReset()` : seulement depuis ERROR, véhicule à l'arrêt, capteurs valides.
- L'application ne connaît **que** des interfaces : pas de `#include "hal/virtual_*.hpp"` dedans.
- ✅ Le scénario de démonstration de `main.cpp` raconte l'histoire attendue. Commit : `feat: ECU controller`.

## Étape 7 — Gestion du temps et des valeurs hors limites (3 h)
- Trois mécanismes distincts, à ne pas confondre : **anti-rebond** (filtre les glitches),
  **timeout** (capteur absent ≥ 500 ms → critique), **temps de stabilité** (récupération).
- Mode `--realtime` : `std::chrono::steady_clock` + `std::this_thread::sleep_until`.
- ✅ Une micro-coupure de 2 cycles ne change pas l'état ; 500 ms d'absence mène à ERROR.

## Étape 8 — CMake (2 h)
- Bibliothèque `ecu_core` (tout sauf `main`), exécutable `ecu_sim`, exécutable `ecu_tests`.
- Options `ECU_BUILD_TESTS`, `ECU_COVERAGE` ; warnings `-Wall -Wextra -Wpedantic -Wconversion`.
- ✅ `cmake -S . -B build && cmake --build build` sans aucun warning.

## Étape 9 — Tests unitaires (8 h)
- Ordre conseillé : validator → diagnostic → sensor service → machine à états.
- Pour chaque règle : cas nominal, **bornes exactes**, cas hors limite, cas interdit.
- Banc `Rig` : assemble HAL simulée + services + application et avance l'horloge cycle par cycle.
- Le framework `minitest.hpp` évite toute dépendance ; migration GoogleTest possible via
  `FetchContent` (les macros `TEST/CHECK_EQ` ressemblent à `TEST/EXPECT_EQ`).
- ✅ `ctest` : 39/39. Commit : `test: unit tests for validator, diagnostic, services, state machine`.

## Étape 10 — Couverture (2 h)
- `cmake -S . -B build-cov -DECU_COVERAGE=ON && cmake --build build-cov --target coverage`
- Lire le rapport : chercher les **branches** non prises, écrire le test manquant ou justifier
  (ex. `return "Unknown"` inatteignable).
- ✅ Chiffres relevés dans `docs/test_plan.md` (ne jamais inventer un pourcentage).

## Étape 11 — Documentation (4 h)
- `docs/architecture.md` : couches, **diagramme de classes UML**, **diagramme de séquence**,
  correspondance AUTOSAR. Mermaid s'affiche directement sur GitHub.
- `docs/state_machine.md`, `docs/test_plan.md` (ID, scénario, attendu, test, résultat).
- ✅ Chaque ligne de doc correspond au code réel (relire après chaque modification).

## Étape 12 — Portfolio (3 h)
- README : contexte, fonctionnalités, build, extrait d'exécution, couverture, limites.
- CI GitHub Actions (`.github/workflows/ci.yml`) : badge vert dans le README.
- Capture d'écran / GIF du terminal pendant la démonstration.
- ✅ Un inconnu clone le dépôt et obtient les mêmes résultats en 3 commandes.

## Étape 13 — Préparer l'entretien
Sois capable d'expliquer :
1. Pourquoi des interfaces entre couches (testabilité, portage STM32/ESP32).
2. Différence entre anti-rebond, timeout et temps de stabilité.
3. Pourquoi ERROR n'est jamais quitté automatiquement.
4. Ce qui est « inspiré d'AUTOSAR » (SWC, Dem, Os cyclique, ports) et ce qui ne l'est pas (RTE, ARXML, NvM, Com).
5. Ce que la couverture prouve (code exécuté) et ne prouve pas (absence de défaut).
6. Comment tu porterais la HAL sur ton STM32 (CAN pour les capteurs, GPIO/PWM pour les actionneurs).

## Extensions possibles (bonus)
Stack CAN simulée (trames avec timeout par message), NvM simulé (persistance des DTC),
fichier de scénarios CSV, mesure de jitter en mode temps réel, port FreeRTOS sur STM32.
