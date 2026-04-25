cmake_minimum_required(VERSION 3.20)
project(wost_toy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WOST_TOY_USE_OPENQMC "Enable OpenQMC backend" ON)

if(WOST_TOY_USE_OPENQMC)
  set(OPENQMC_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
  set(OPENQMC_BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(OPENQMC_ENABLE_BINARY OFF CACHE BOOL "" FORCE)
  set(OPENQMC_ARCH_TYPE Scalar CACHE STRING "" FORCE)

  add_subdirectory(external/openqmc EXCLUDE_FROM_ALL)

  target_compile_definitions(wost_toy_lib PUBLIC WOST_TOY_USE_OPENQMC=1)
  target_link_libraries(wost_toy_lib PUBLIC OpenQMC::OpenQMC)
endif()

add_executable(mixed_boundary_benchmark
    experiments/mixed_boundary_benchmark.cpp
)

target_include_directories(mixed_boundary_benchmark PRIVATE include)

if(MSVC)
    target_compile_options(mixed_boundary_benchmark PRIVATE /W4 /permissive-)
else()
    target_compile_options(mixed_boundary_benchmark PRIVATE -Wall -Wextra -pedantic -O3)
endif()
