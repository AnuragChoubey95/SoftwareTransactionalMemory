cmake_minimum_required(VERSION 3.10)
project(SoftwareTransactionalMemory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_SHARED_LINKER_FLAGS "")

add_compile_options(-Wall -Wextra -Werror -pedantic)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Core STM library
add_library(tl2_core
    ${CMAKE_SOURCE_DIR}/src/gvc.cpp
    ${CMAKE_SOURCE_DIR}/src/vlock.cpp
)

# --- Unit tests ------------------------------------------------------------
enable_testing()
find_package(GTest REQUIRED)

add_executable(tl2_tests
    tests/test_gvc.cpp
    tests/test_vlock.cpp
)

target_link_libraries(tl2_tests
    PRIVATE
        tl2_core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tl2_tests)
# ---------------------------------------------------------------------------
