cmake_minimum_required(VERSION 3.16)
project(cmdx VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(nlohmann_json REQUIRED)

set(CORE_SOURCES
    core/json_parser.cpp
    core/tool_registry.cpp
    core/command_builder.cpp
)

set(CORE_HEADERS
    core/tool_def.hpp
    core/json_parser.hpp
    core/tool_registry.hpp
    core/command_builder.hpp
)

set(PLATFORM_SOURCES
    platform/linux/cli.cpp
)

set(PLATFORM_HEADERS
    platform/linux/cli.hpp
)

set(UI_SOURCES
    ui/option_widget.cpp
    ui/options_form.cpp
    ui/main_window.cpp
)

set(UI_HEADERS
    ui/option_widget.hpp
    ui/options_form.hpp
    ui/main_window.hpp
)

add_executable(cmdx
    app/main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${PLATFORM_SOURCES}
    ${PLATFORM_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
)

target_include_directories(cmdx PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(cmdx PRIVATE Qt6::Widgets nlohmann_json::nlohmann_json)

file(COPY ${CMAKE_SOURCE_DIR}/commands DESTINATION ${CMAKE_BINARY_DIR})
