cmake_minimum_required(VERSION 3.20)
project(wost_toy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep the default dependency-free so the toy still builds without initialized
# submodules. Enable this for fair Owen-scrambled Sobol comparisons.
option(WOST_TOY_USE_OPENQMC "Enable OpenQMC-backed QMC samplers" OFF)

add_executable(mixed_boundary_benchmark
    experiments/mixed_boundary_benchmark.cpp
)

target_include_directories(mixed_boundary_benchmark PRIVATE include)

if(WOST_TOY_USE_OPENQMC)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/openqmc/CMakeLists.txt")
        message(FATAL_ERROR
            "WOST_TOY_USE_OPENQMC=ON, but external/openqmc is not initialized.\n"
            "Run: git submodule update --init --recursive external/openqmc")
    endif()

    set(OPENQMC_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(OPENQMC_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(OPENQMC_ENABLE_BINARY OFF CACHE BOOL "" FORCE)
    set(OPENQMC_ARCH_TYPE Scalar CACHE STRING "" FORCE)

    add_subdirectory(external/openqmc EXCLUDE_FROM_ALL)

    target_compile_definitions(mixed_boundary_benchmark PRIVATE WOST_TOY_USE_OPENQMC=1)
    target_link_libraries(mixed_boundary_benchmark PRIVATE OpenQMC::OpenQMC)
endif()

if(MSVC)
    target_compile_options(mixed_boundary_benchmark PRIVATE /W4 /permissive-)
else()
    target_compile_options(mixed_boundary_benchmark PRIVATE -Wall -Wextra -pedantic -O3)
endif()
