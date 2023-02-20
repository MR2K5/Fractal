cmake_minimum_required(VERSION 3.23)

project(Fractal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

add_library(common INTERFACE "")

set(USE_SANITIZER "" CACHE STRING "Enabled sanitizers")

target_compile_options(common INTERFACE $<$<BOOL:USE_SANITIZER>: -fsanitize=${USE_SANITIZER}> -march=native -gdwarf-4)
target_link_options(common INTERFACE $<$<BOOL:USE_SANITIZER>: -fsanitize=${USE_SANITIZER}> -march=native -gdwarf-4)

target_compile_options(common INTERFACE $<$<CXX_COMPILER_ID:clang>: -stdlib=libc++>)
target_link_options(common INTERFACE $<$<CXX_COMPILER_ID:clang>: -stdlib=libc++>)

target_include_directories(common INTERFACE include/)

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

