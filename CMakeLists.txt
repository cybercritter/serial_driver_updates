cmake_minimum_required(VERSION 3.16)

project(serial_driver_updates LANGUAGES C CXX)

option(SERIAL_DRIVER_ENABLE_TESTS "Build unit tests" ON)
option(SERIAL_DRIVER_ENABLE_COVERAGE "Enable coverage flags for tests" OFF)

add_library(x17v358 INTERFACE)

target_include_directories(x17v358
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(x17v358 INTERFACE c_std_11)

if(SERIAL_DRIVER_ENABLE_TESTS)
    include(CTest)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(x17v358_tests
        tests/x17v358_regs_test.cpp
    )
    target_link_libraries(x17v358_tests
        PRIVATE
            x17v358
            GTest::gtest_main
    )
    target_compile_features(x17v358_tests PRIVATE cxx_std_17)

    if(SERIAL_DRIVER_ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(x17v358_tests PRIVATE -O0 -g --coverage)
            target_link_options(x17v358_tests PRIVATE --coverage)
        else()
            message(WARNING "Coverage requested, but compiler does not support --coverage flags")
        endif()
    endif()

    include(GoogleTest)
    gtest_discover_tests(x17v358_tests)

    if(SERIAL_DRIVER_ENABLE_COVERAGE)
        find_program(GCOVR_EXECUTABLE gcovr)
        if(GCOVR_EXECUTABLE)
            add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${GCOVR_EXECUTABLE}
                        --root ${CMAKE_SOURCE_DIR}
                        --filter ${CMAKE_SOURCE_DIR}/include
                        --filter ${CMAKE_SOURCE_DIR}/tests
                        --exclude-directories ${CMAKE_BINARY_DIR}
                        --txt
                        --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Running tests and generating gcovr text coverage report"
                VERBATIM
            )
        else()
            message(STATUS "gcovr not found: 'coverage' target will not be available")
        endif()
    endif()
endif()
