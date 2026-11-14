cmake_minimum_required(VERSION 3.10)

project(seatManager VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(EXECUTABLE_SOURCES
    src/main.cpp
    src/core.cpp
    src/mainwindow.cpp
    src/studentdialog.cpp
    src/ruledialog.cpp
    src/seattable.cpp
)
file(GLOB COMMON_SOURCES src/utils/*.cpp src/utils/*.c)
file(GLOB COMMON_HANDERS src/include/*.hpp src/include/*.h)

message(STATUS "COMMON_SOURCES = ${COMMON_SOURCES}")
add_executable(${PROJECT_NAME} ${COMMON_HANDERS} ${EXECUTABLE_SOURCES} ${COMMON_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src/include)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)