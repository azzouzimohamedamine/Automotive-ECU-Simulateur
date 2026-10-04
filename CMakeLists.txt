cmake_minimum_required(VERSION 3.16)
project(AutomotiveECUSimulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ECU_BUILD_TESTS "Build unit tests" ON)
option(ECU_COVERAGE "Instrument with gcov (GCC/Clang)" OFF)

add_library(ecu_core STATIC
    src/common/types.cpp
    src/hal/virtual_sensors.cpp
    src/hal/virtual_actuators.cpp
    src/services/sensor_validator.cpp
    src/services/sensor_service.cpp
    src/services/diagnostic.cpp
    src/application/ecu_controller.cpp
)
target_include_directories(ecu_core PUBLIC include)

if(MSVC)
    target_compile_options(ecu_core PUBLIC /W4)
else()
    target_compile_options(ecu_core PUBLIC -Wall -Wextra -Wpedantic -Wconversion)
endif()

if(ECU_COVERAGE)
    target_compile_options(ecu_core PUBLIC --coverage -O0 -g)
    target_link_options(ecu_core PUBLIC --coverage)
endif()

add_executable(ecu_sim src/main.cpp)
target_link_libraries(ecu_sim PRIVATE ecu_core)

if(ECU_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
