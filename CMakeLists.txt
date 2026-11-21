cmake_minimum_required(VERSION 3.21)
project(VentuSample VERSION 0.2.0 LANGUAGES CXX)

option(VENTU_BUILD_DESKTOP "Build the native Qt desktop workstation when Qt 6 is available" ON)
option(VENTU_BUILD_TESTS "Build deterministic generator tests" ON)
option(VENTU_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer on GCC/Clang" OFF)
set(REAPER_SOURCE_DIR "" CACHE PATH "Optional local Reaper source tree for native expression integration")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(REAPER_SOURCE_DIR)
    if(NOT EXISTS "${REAPER_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "REAPER_SOURCE_DIR must point to a Reaper source checkout")
    endif()
    set(_VENTU_REAPER_BUILD_TESTS_WAS_DEFINED FALSE)
    if(DEFINED REAPER_BUILD_TESTS)
        set(_VENTU_REAPER_BUILD_TESTS_WAS_DEFINED TRUE)
        set(_VENTU_REAPER_BUILD_TESTS_PREVIOUS "${REAPER_BUILD_TESTS}")
    endif()
    set(REAPER_BUILD_TESTS OFF)
    add_subdirectory("${REAPER_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_reaper" EXCLUDE_FROM_ALL)
    if(_VENTU_REAPER_BUILD_TESTS_WAS_DEFINED)
        set(REAPER_BUILD_TESTS "${_VENTU_REAPER_BUILD_TESTS_PREVIOUS}")
    else()
        unset(REAPER_BUILD_TESTS)
    endif()
endif()

add_library(ventusample_core STATIC
    src/generator.cpp
    src/generator.hpp
    src/plugin_manager.cpp
    src/plugin_manager.hpp
    src/stochastic_router.hpp
    src/stochastic_router.cpp
    src/builtin_plugins.cpp
    src/quantum_audio.cpp
    src/quantum_audio.hpp
    src/atomic_quantum.cpp
    src/atomic_quantum.hpp
    src/budget_planner.cpp
    src/budget_planner.hpp
    src/reaper_expression.cpp
    src/reaper_expression.hpp
)
target_include_directories(ventusample_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(ventusample_core PUBLIC cxx_std_20)
if(REAPER_SOURCE_DIR)
    target_link_libraries(ventusample_core PUBLIC reaper_ffi)
    target_compile_definitions(ventusample_core PUBLIC VENTU_HAS_REAPER=1)
    message(STATUS "VentuSample: Reaper native C17 expression bridge enabled")
else()
    message(STATUS "VentuSample: Reaper bridge disabled (set REAPER_SOURCE_DIR to enable)")
endif()
if(MSVC)
    target_compile_options(ventusample_core PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(ventusample_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(VENTU_BUILD_DESKTOP)
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if(Qt6Widgets_FOUND)
        set(CMAKE_AUTOMOC ON)
        add_executable(VentuSample WIN32 MACOSX_BUNDLE
            src/main.cpp
            src/mainwindow.cpp
            src/mainwindow.hpp
            src/ui_motion.cpp
            src/ui_motion.hpp
            src/ui_ux.cpp
            src/ui_ux.hpp
            src/quantum_audio_lab.cpp
            src/quantum_audio_lab.hpp
            src/budget_planner_dialog.cpp
            src/budget_planner_dialog.hpp
        )
        target_link_libraries(VentuSample PRIVATE ventusample_core Qt6::Widgets)
        target_include_directories(VentuSample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
        if(VENTU_BUILD_TESTS)
            add_executable(ventusample_ui_motion_tests
                tests/test_ui_motion.cpp
                src/ui_motion.cpp
                src/ui_motion.hpp
            )
            target_link_libraries(ventusample_ui_motion_tests PRIVATE Qt6::Widgets)
            target_include_directories(ventusample_ui_motion_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
            add_executable(ventusample_ui_ux_tests
                tests/test_ui_ux_40026.cpp
                src/ui_ux.cpp
                src/ui_ux.hpp
            )
            target_link_libraries(ventusample_ui_ux_tests PRIVATE Qt6::Widgets)
            target_include_directories(ventusample_ui_ux_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
        endif()
        if(WIN32)
            target_compile_definitions(VentuSample PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00 WINVER=0x0A00)
        endif()
        set_target_properties(VentuSample PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "VentuSample"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.ivan-vhs.ventusample"
            OUTPUT_NAME "VentuSample"
        )
        message(STATUS "VentuSample desktop target enabled (Qt ${Qt6_VERSION})")
    else()
        message(WARNING "Qt 6 Widgets not found: the DSP core and tests will build; install Qt 6 Widgets to build the desktop application.")
    endif()
endif()

if(VENTU_ENABLE_SANITIZERS AND NOT MSVC)
    foreach(target ventusample_core)
        target_compile_options(${target} PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PUBLIC -fsanitize=address,undefined)
    endforeach()
endif()

if(VENTU_BUILD_TESTS)
    enable_testing()
    if(VENTU_BUILD_DESKTOP AND Qt6Widgets_FOUND)
        add_test(NAME ventusample-ui-motion COMMAND ventusample_ui_motion_tests)
        add_test(NAME ventusample-ui-ux-40026 COMMAND ventusample_ui_ux_tests)
    endif()
    add_executable(ventusample_tests tests/test_generator.cpp)
    target_link_libraries(ventusample_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-generator COMMAND ventusample_tests)
    add_executable(ventusample_plugin_tests tests/test_plugins.cpp)
    target_link_libraries(ventusample_plugin_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-plugin-host COMMAND ventusample_plugin_tests)
    add_executable(ventusample_stochastic_tests tests/test_stochastic_router.cpp)
    target_link_libraries(ventusample_stochastic_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-stochastic-routing COMMAND ventusample_stochastic_tests)
    add_executable(ventusample_quantum_audio_tests tests/test_quantum_audio.cpp)
    target_link_libraries(ventusample_quantum_audio_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-quantum-audio COMMAND ventusample_quantum_audio_tests)
    add_executable(ventusample_budget_tests tests/test_budget_planner.cpp)
    target_link_libraries(ventusample_budget_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-budget-planner COMMAND ventusample_budget_tests)
    add_executable(ventusample_atomic_quantum_tests tests/test_atomic_quantum.cpp)
    target_link_libraries(ventusample_atomic_quantum_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-atomic-quantum COMMAND ventusample_atomic_quantum_tests)
    add_executable(ventusample_reaper_expression_tests tests/test_reaper_expression.cpp)
    target_link_libraries(ventusample_reaper_expression_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-reaper-expression-bridge COMMAND ventusample_reaper_expression_tests)
    find_package(Python3 QUIET COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_test(NAME ventusample-contract COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_project_contract.py)
        add_test(NAME ventusample-manifest-fixture COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_generated_fixture.py)
        set_tests_properties(ventusample-manifest-fixture PROPERTIES DEPENDS ventusample-generator)
    endif()
endif()
