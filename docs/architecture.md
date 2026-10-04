# Architecture

> Architecture **pédagogique inspirée d'AUTOSAR**. Ce n'est pas une implémentation
> (ni certifiée, ni conforme) de la norme AUTOSAR Classic.

## 1. Couches

```mermaid
flowchart TB
    SCN["Scénario / injection de défauts (main.cpp, tests)"]
    APP["Application — EcuController<br/>machine à états, décisions, commandes"]
    SRV["Services — SensorService, sensor_validator,<br/>DiagnosticService, IClock"]
    HAL["Abstraction matérielle — ISensorHal / IActuatorHal<br/>VirtualSensorHal, VirtualActuatorHal"]
    SCN --> HAL
    APP --> SRV
    SRV --> HAL
    APP --> HAL
```

Règle : une couche ne dépend que de la couche inférieure, et uniquement via des
**interfaces** (`ISensorHal`, `IActuatorHal`, `IClock`). Remplacer la HAL simulée
par une HAL STM32/ESP32 ne demanderait aucune modification de l'application.

## 2. Diagramme UML de classes

```mermaid
classDiagram
    class EcuController {
        -EcuState state_
        -Snapshot snapshot_
        +step()
        +requestReset() bool
        +state() EcuState
        +transitions() vector~Transition~
        +commandFor(EcuState)$ ActuatorCommand
        -runInit()
        -evaluateFaults()
        -supervise()
        -enterState(EcuState, reason)
    }
    class SensorService {
        +selfTest() bool
        +update() Snapshot
    }
    class DiagnosticService {
        +report(FaultCode, failed)
        +isActive(FaultCode) bool
        +anyActive(Severity) bool
        +clearAll()
        +log() vector~DiagEvent~
    }
    class ISensorHal {
        <<interface>>
        +isReady() bool
        +read(SensorId) RawReading
    }
    class IActuatorHal {
        <<interface>>
        +apply(ActuatorCommand)
    }
    class IClock {
        <<interface>>
        +nowMs() uint32
    }
    class VirtualSensorHal {
        +setValue(SensorId, double)
        +setMissing(SensorId, bool)
        +setReady(bool)
    }
    class VirtualActuatorHal {
        +last() ActuatorCommand
        +applyCount() size_t
    }
    class SimClock {
        +advance(ms)
    }

    ISensorHal <|.. VirtualSensorHal
    IActuatorHal <|.. VirtualActuatorHal
    IClock <|.. SimClock
    EcuController --> SensorService : utilise
    EcuController --> DiagnosticService : utilise
    EcuController --> IActuatorHal : commande
    EcuController --> IClock
    SensorService --> ISensorHal : lit
    SensorService --> IClock
    DiagnosticService --> IClock
```

## 3. Diagramme de séquence — un cycle de 100 ms (cas : surchauffe)

```mermaid
sequenceDiagram
    participant S as Ordonnanceur (main)
    participant E as EcuController
    participant SS as SensorService
    participant H as VirtualSensorHal
    participant D as DiagnosticService
    participant A as VirtualActuatorHal

    S->>E: step()
    E->>SS: update()
    loop pour chaque capteur
        SS->>H: read(id)
        H-->>SS: RawReading (valeur, disponible)
        SS->>SS: validate() + durée d'absence
    end
    SS-->>E: Snapshot
    E->>D: report(D007, température > 105 °C ?)
    Note over D: anti-rebond : confirmé après 3 cycles consécutifs
    E->>D: anyActive(Critical)
    D-->>E: false
    E->>D: anyActive(Recoverable)
    D-->>E: true
    E->>E: enterState(DEGRADED)
    E->>A: apply(commandFor(DEGRADED))
    S->>S: clock.advance(100 ms)
```

## 4. Correspondance avec les concepts AUTOSAR (à titre pédagogique)

| Ce projet | Concept AUTOSAR Classic proche | Écart assumé |
|---|---|---|
| `ISensorHal` / `IActuatorHal` + implémentations virtuelles | ECU Abstraction / MCAL (accès matériel) | Pas de drivers réels, pas d'IoHwAb standardisé |
| `EcuController` | Composant logiciel applicatif (SWC) | Pas de description ARXML |
| `EcuController::step()` | Runnable cyclique (100 ms) | Appelé par une boucle simple, pas par un OS OSEK/AUTOSAR |
| `DiagnosticService` (anti-rebond, gravité, journal) | Dem (Diagnostic Event Manager) simplifié | Pas d'UDS, pas de snapshot/extended data, codes fictifs |
| `IClock`, boucle périodique, timeouts | Os counters/alarms, timeouts de signal | Temps simulé déterministe |
| Injection des dépendances dans `main` / tests | Rôle du RTE (câblage des ports) | Câblage manuel, pas de génération de code |
| Non implémenté | NvM, Com/CAN stack, BswM, EcuM, SafeE, Det | Hors périmètre |
