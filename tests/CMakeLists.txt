cmake_minimum_required(VERSION 3.16)
project(psycho_client_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Test Core Network Widgets)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/diary
)

add_executable(test_client_logic
    test_client_logic.cpp
    ../src/network/ErrorHelper.h
)

target_link_libraries(test_client_logic PRIVATE Qt6::Test Qt6::Core Qt6::Network Qt6::Widgets)

add_test(NAME test_client_logic COMMAND test_client_logic)
