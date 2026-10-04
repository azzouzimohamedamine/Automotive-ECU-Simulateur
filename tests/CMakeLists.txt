add_executable(ecu_tests
    test_main.cpp
    test_sensor_validator.cpp
    test_sensor_service.cpp
    test_diagnostic.cpp
    test_state_machine.cpp
)
target_link_libraries(ecu_tests PRIVATE ecu_core)
add_test(NAME ecu_tests COMMAND ecu_tests)

if(ECU_COVERAGE)
    find_program(GCOVR_EXE gcovr)
    if(GCOVR_EXE)
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_EXE} --root ${PROJECT_SOURCE_DIR}
                    --filter ${PROJECT_SOURCE_DIR}/src
                    --filter ${PROJECT_SOURCE_DIR}/include
                    --exclude ${PROJECT_SOURCE_DIR}/src/main.cpp
                    --txt --print-summary --html-details ${CMAKE_BINARY_DIR}/coverage.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ecu_tests
            COMMENT "Running tests and generating coverage report")
    else()
        message(WARNING "gcovr not found: pip install gcovr")
    endif()
endif()
