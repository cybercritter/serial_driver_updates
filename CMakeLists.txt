cmake_minimum_required(VERSION 3.16)

project(serial_driver_updates LANGUAGES C CXX)

option(SERIAL_DRIVER_ENABLE_TESTS "Build unit tests" ON)
option(SERIAL_DRIVER_ENABLE_COVERAGE "Enable coverage flags for tests" ON)

add_library(x17v358 STATIC source/serial_driver.c)
target_include_directories(x17v358
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(x17v358 PUBLIC c_std_11)

add_executable(serial_driver_updates source/main.c)
target_link_libraries(serial_driver_updates PRIVATE x17v358)

if(SERIAL_DRIVER_ENABLE_TESTS)
    include(CTest)
    include(FetchContent)

    target_compile_definitions(x17v358 PRIVATE SERIAL_DRIVER_TESTING=1)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(x17v358_tests
        tests/serial_driver_test.cpp
        tests/x17v358_regs_test.cpp
    )
    target_link_libraries(x17v358_tests
        PRIVATE
            x17v358
            GTest::gtest_main
    )
    target_compile_features(x17v358_tests PRIVATE cxx_std_17)
    target_compile_definitions(x17v358_tests PRIVATE SERIAL_DRIVER_TESTING=1)

    if(SERIAL_DRIVER_ENABLE_COVERAGE)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(x17v358 PRIVATE -O0 -g --coverage)
            target_link_options(x17v358 PRIVATE --coverage)
            target_compile_options(serial_driver_updates PRIVATE -O0 -g --coverage)
            target_link_options(serial_driver_updates PRIVATE --coverage)
        endif()
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
        set(GCOVR_RUNNER)
        set(GCOVR_GCOV_EXECUTABLE_ARGS)
        find_program(GCOVR_EXECUTABLE NAMES gcovr)
        if(GCOVR_EXECUTABLE)
            set(GCOVR_RUNNER ${GCOVR_EXECUTABLE})
        else()
            find_package(Python3 COMPONENTS Interpreter QUIET)
            if(Python3_Interpreter_FOUND)
                set(GCOVR_RUNNER ${Python3_EXECUTABLE} -m gcovr)
            endif()
        endif()

        if(CMAKE_C_COMPILER_ID MATCHES "Clang")
            find_program(LLVM_COV_EXECUTABLE NAMES llvm-cov llvm-cov-21)
            if(LLVM_COV_EXECUTABLE)
                set(GCOVR_GCOV_EXECUTABLE_ARGS --gcov-executable "${LLVM_COV_EXECUTABLE} gcov")
            endif()
        endif()

        if(GCOVR_RUNNER)
            add_custom_target(coverage
                DEPENDS x17v358_tests serial_driver_updates
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${GCOVR_RUNNER}
                        --root ${CMAKE_SOURCE_DIR}
                    --object-directory ${CMAKE_BINARY_DIR}
                        ${GCOVR_GCOV_EXECUTABLE_ARGS}
                    --filter ${CMAKE_SOURCE_DIR}/source/serial_driver\.c$
                    --filter ${CMAKE_SOURCE_DIR}/source/.*
                    --filter ${CMAKE_SOURCE_DIR}/include/.*
                        --exclude ${CMAKE_SOURCE_DIR}/tests
                    --exclude ${CMAKE_BINARY_DIR}/_deps
                        --txt
                        --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Running tests and generating gcovr text coverage report"
                VERBATIM
            )
        else()
            add_custom_target(coverage
                COMMAND ${CMAKE_COMMAND} -E echo "gcovr not found. Install gcovr or make it available via Python (python -m gcovr)."
                COMMAND ${CMAKE_COMMAND} -E false
                COMMENT "Coverage toolchain unavailable"
                VERBATIM
            )
        endif()
    else()
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage is disabled. Reconfigure with -DSERIAL_DRIVER_ENABLE_COVERAGE=ON"
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "Coverage target disabled by SERIAL_DRIVER_ENABLE_COVERAGE=OFF"
            VERBATIM
        )
    endif()
endif()
