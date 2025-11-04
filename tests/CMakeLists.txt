cmake_minimum_required(VERSION 3.16)

# Enable test discovery
enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

# ---- GVC tests ----
add_executable(test_gvc
    ../src/gvc.cpp
    test_gvc.cpp
)
target_include_directories(test_gvc PRIVATE ../include)
target_link_libraries(test_gvc PRIVATE GTest::gtest_main)
gtest_discover_tests(test_gvc)

# ---- VLOCK tests ----
add_executable(test_vlock
    ../src/vlock.cpp
    test_vlock.cpp
)
target_include_directories(test_vlock PRIVATE ../include)
target_link_libraries(test_vlock PRIVATE GTest::gtest_main)
gtest_discover_tests(test_vlock)
