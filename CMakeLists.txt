cmake_minimum_required(VERSION 3.21)
project(VentuSample VERSION 0.2.0 LANGUAGES CXX)

option(VENTU_BUILD_DESKTOP "Build the native Qt desktop workstation when Qt 6 is available" ON)
option(VENTU_BUILD_TESTS "Build deterministic generator tests" ON)
option(VENTU_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer on GCC/Clang" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ventusample_core STATIC
    src/generator.cpp
    src/generator.hpp
    src/plugin_manager.cpp
    src/plugin_manager.hpp
    src/builtin_plugins.cpp
    src/quantum_audio.cpp
    src/quantum_audio.hpp
)
target_include_directories(ventusample_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(ventusample_core PUBLIC cxx_std_20)
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
            src/quantum_audio_lab.cpp
            src/quantum_audio_lab.hpp
        )
        target_link_libraries(VentuSample PRIVATE ventusample_core Qt6::Widgets)
        target_include_directories(VentuSample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
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
    add_executable(ventusample_tests tests/test_generator.cpp)
    target_link_libraries(ventusample_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-generator COMMAND ventusample_tests)
    add_executable(ventusample_plugin_tests tests/test_plugins.cpp)
    target_link_libraries(ventusample_plugin_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-plugin-host COMMAND ventusample_plugin_tests)
    add_executable(ventusample_quantum_audio_tests tests/test_quantum_audio.cpp)
    target_link_libraries(ventusample_quantum_audio_tests PRIVATE ventusample_core)
    add_test(NAME ventusample-quantum-audio COMMAND ventusample_quantum_audio_tests)
    find_package(Python3 QUIET COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_test(NAME ventusample-contract COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_project_contract.py)
        add_test(NAME ventusample-manifest-fixture COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_generated_fixture.py)
        set_tests_properties(ventusample-manifest-fixture PROPERTIES DEPENDS ventusample-generator)
    endif()
endif()
