# MCST Elbrus E2K Linux toolchain.
#
# Cross-build usage:
#   E2K_CC=/absolute/path/to/e2k-gcc \
#   E2K_CXX=/absolute/path/to/e2k-g++ \
#   E2K_SYSROOT=/path/to/e2k-sysroot \
#   cmake -S . -B build-e2k -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/elbrus-e2k.cmake
#
# Do not add a generic -march/-mcpu flag here. MCST compiler packages are
# target/model-specific; use the flags and sysroot shipped with that package.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR e2k)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT CMAKE_C_COMPILER AND DEFINED ENV{E2K_CC} AND NOT "$ENV{E2K_CC}" STREQUAL "")
  set(CMAKE_C_COMPILER "$ENV{E2K_CC}" CACHE FILEPATH "MCST E2K C compiler")
endif()

if(NOT CMAKE_CXX_COMPILER AND DEFINED ENV{E2K_CXX} AND NOT "$ENV{E2K_CXX}" STREQUAL "")
  set(CMAKE_CXX_COMPILER "$ENV{E2K_CXX}" CACHE FILEPATH "MCST E2K C++ compiler")
endif()

if(NOT CMAKE_C_COMPILER AND NOT CMAKE_CXX_COMPILER)
  message(FATAL_ERROR
    "Set E2K_CC and/or E2K_CXX to the MCST LCC compiler (or its documented GCC-compatible driver).")
endif()

if(DEFINED ENV{E2K_SYSROOT} AND NOT "$ENV{E2K_SYSROOT}" STREQUAL "")
  set(CMAKE_SYSROOT "$ENV{E2K_SYSROOT}")
  list(PREPEND CMAKE_FIND_ROOT_PATH "$ENV{E2K_SYSROOT}")
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

if(DEFINED ENV{E2K_CROSSCOMPILING_EMULATOR} AND NOT "$ENV{E2K_CROSSCOMPILING_EMULATOR}" STREQUAL "")
  separate_arguments(_e2k_emulator NATIVE_COMMAND "$ENV{E2K_CROSSCOMPILING_EMULATOR}")
  set(CMAKE_CROSSCOMPILING_EMULATOR "${_e2k_emulator}")
endif()
