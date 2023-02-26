cmake_minimum_required(VERSION 3.23)

project(Fractal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

find_package(GTest REQUIRED)
enable_testing()

add_library(common INTERFACE "")

set(USE_SANITIZER "" CACHE STRING "Enabled sanitizers")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "g++ or clang++")
    target_compile_options(common INTERFACE $<$<BOOL:USE_SANITIZER>: -fsanitize=${USE_SANITIZER}> -march=native -gdwarf-4 -Wall -Wextra -pedantic)
    target_link_options(common INTERFACE $<$<BOOL:USE_SANITIZER>: -fsanitize=${USE_SANITIZER}> -march=native -gdwarf-4 -Wall -Wextra)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(common INTERFACE $<$<CXX_COMPILER_ID:clang>: -stdlib=libc++>)
    target_link_options(common INTERFACE $<$<CXX_COMPILER_ID:clang>: -stdlib=libc++>)
endif()

target_include_directories(common INTERFACE include/)

option(ENABLE_XRAY FALSE)
if (ENABLE_XRAY)
    target_compile_options(common INTERFACE -fxray-instrument)
    target_link_options(common INTERFACE -fxray-instrument)
endif()

option(ENABLE_AVX512 FALSE)
if (ENABLE_AVX512)
    target_compile_definitions(common INTERFACE HAS_AVX512)
else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(common INTERFACE -mno-avx512f)
        target_link_options(common INTERFACE -mno-avx512f)
    endif()
endif()


add_executable(Viewer "")
target_link_libraries(Viewer PRIVATE common)


add_subdirectory(src)
add_subdirectory(include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

find_package(Eigen3 REQUIRED)

target_include_directories(Viewer PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(Viewer PRIVATE ${GTKMM_LIBRARY_DIRS})
target_link_libraries(Viewer PRIVATE ${GTKMM_LIBRARIES} Eigen3::Eigen)

