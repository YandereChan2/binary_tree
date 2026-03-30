cmake_minimum_required(VERSION 3.14)  # FetchContent 需要 3.14+
project(binary_tree)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # 设置默认运行时库为动态链接
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE  # 添加这一行
)

# 重要：强制 Google Test 也使用动态运行时库
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(test_runner
    test/main.cpp
    test/test.cpp
)

target_include_directories(test_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_runner GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_runner)