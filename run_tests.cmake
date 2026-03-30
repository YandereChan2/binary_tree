cmake_minimum_required(VERSION 3.14)

message(STATUS "========================================")
message(STATUS "binary_tree.h Unit Test Runner")
message(STATUS "========================================")

set(BUILD_DIR "${CMAKE_CURRENT_LIST_DIR}/out/build")
set(CONFIG "Debug")

# 清理构建目录
if(EXISTS ${BUILD_DIR})
    message(STATUS "Cleaning build directory...")
    file(REMOVE_RECURSE ${BUILD_DIR})
endif()

# 创建构建目录
file(MAKE_DIRECTORY ${BUILD_DIR})

# 配置 CMake
message(STATUS "\nConfiguring CMake...")
execute_process(
    COMMAND ${CMAKE_COMMAND} 
        ${CMAKE_CURRENT_LIST_DIR}
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>DLL
    WORKING_DIRECTORY ${BUILD_DIR}
    RESULT_VARIABLE CONFIG_RESULT
)

if(NOT CONFIG_RESULT EQUAL 0)
    message(FATAL_ERROR "CMake configuration failed")
endif()
message(STATUS "CMake configuration completed")

# 构建项目
message(STATUS "\nBuilding project...")
execute_process(
    COMMAND ${CMAKE_COMMAND} --build . --config ${CONFIG}
    WORKING_DIRECTORY ${BUILD_DIR}
    RESULT_VARIABLE BUILD_RESULT
)

if(NOT BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "Build failed")
endif()
message(STATUS "Build completed")

# 运行测试
message(STATUS "\nRunning tests...")
message(STATUS "----------------------------------------")

execute_process(
    COMMAND ${BUILD_DIR}/${CONFIG}/test_runner.exe
    RESULT_VARIABLE TEST_RESULT
)

message(STATUS "----------------------------------------")

if(TEST_RESULT EQUAL 0)
    message(STATUS "\n✓ All tests passed!")
else()
    message(FATAL_ERROR "\n✗ Some tests failed!")
endif()