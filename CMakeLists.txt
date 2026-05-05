cmake_minimum_required(VERSION 3.16)
project(cmdx VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(nlohmann_json REQUIRED)

# ─── Sources ────────────────────────────────────────────────────────────────────

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
    platform/cli.cpp
)

set(PLATFORM_HEADERS
    platform/cli.hpp
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

# ─── Target ─────────────────────────────────────────────────────────────────────

add_executable(cmdx
    app/main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${PLATFORM_SOURCES}
    ${PLATFORM_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
)

include(GNUInstallDirs)

target_compile_definitions(cmdx PRIVATE
    CMDX_VERSION="${PROJECT_VERSION}"
    CMDX_DATA_DIR="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/cmdx"
)

target_include_directories(cmdx PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(cmdx PRIVATE Qt6::Widgets nlohmann_json::nlohmann_json)

# Dev convenience: copy commands into build dir for running in-tree
file(COPY ${CMAKE_SOURCE_DIR}/commands DESTINATION ${CMAKE_BINARY_DIR})

# ─── Install ────────────────────────────────────────────────────────────────────

install(TARGETS cmdx RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install the full command library
install(DIRECTORY commands/ DESTINATION ${CMAKE_INSTALL_DATADIR}/cmdx/commands)
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/cmdx)

if(WIN32)
    # Bundle the MSVC/UCRT runtime explicitly. Qt can do this too, but making
    # CMake responsible for compiler runtime deployment is more predictable in CI.
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)

    qt_generate_deploy_app_script(
        TARGET cmdx
        OUTPUT_SCRIPT cmdx_deploy_script
        NO_COMPILER_RUNTIME
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${cmdx_deploy_script})
endif()

# Create the three GUI alias symlinks
if(UNIX)
    foreach(_alias optionsgui optgui cmdxgui)
        install(CODE "
            set(_link \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${_alias}\")
            message(STATUS \"Creating symlink: \${_link} -> cmdx\")
            execute_process(COMMAND \${CMAKE_COMMAND} -E create_symlink cmdx \${_link})
        ")
    endforeach()
endif()

if(WIN32)
    # On Windows, create .cmd wrapper scripts instead of symlinks
    foreach(_alias optionsgui optgui cmdxgui)
        install(CODE "
            set(_wrapper \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${_alias}.cmd\")
            message(STATUS \"Creating wrapper: \${_wrapper}\")
            file(WRITE \${_wrapper} \"@echo off\\n\\\"%~dp0cmdx.exe\\\" %*\\n\")
        ")
    endforeach()
endif()

# ─── CPack ──────────────────────────────────────────────────────────────────────

set(CPACK_PACKAGE_NAME "cmdx")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multiplatform command reference and execution console")
set(CPACK_PACKAGE_DESCRIPTION "Cmdx is a GUI and terminal command builder. It reads a JSON database of CLI tools and presents an interactive form to construct commands with proper flags, arguments, and validation.")
set(CPACK_PACKAGE_VENDOR "cmdx")
set(CPACK_PACKAGE_CONTACT "snatcha <snatcha@cmdx.dev>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Ishant89op/cmdx")

# ── DEB (.deb) ──────────────────────────────────────────────────────────────────
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "snatcha <snatcha@cmdx.dev>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6 (>= 6.2)")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# ── RPM (.rpm) ──────────────────────────────────────────────────────────────────
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase >= 6.2")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

# ── NSIS (.exe installer for Windows) ────────────────────────────────────────────
set(CPACK_PACKAGE_FILE_NAME "Cmdx-Setup-${PROJECT_VERSION}-win64")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Cmdx")
set(CPACK_PACKAGE_EXECUTABLES "cmdx" "Cmdx")
set(CPACK_CREATE_DESKTOP_LINKS "cmdx")
set(CPACK_NSIS_DISPLAY_NAME "Cmdx")
set(CPACK_NSIS_PACKAGE_NAME "Cmdx ${PROJECT_VERSION}")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ${CMAKE_INSTALL_BINDIR})
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_HELP_LINK "https://github.com/Ishant89op/cmdx")
set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/Ishant89op/cmdx")
set(CPACK_NSIS_CONTACT "snatcha@cmdx.dev")
set(CPACK_NSIS_MENU_LINKS
    "${CMAKE_INSTALL_DATADIR}/doc/cmdx/README.md" "README"
    "https://github.com/Ishant89op/cmdx" "Cmdx Homepage"
)

# ── macOS DragNDrop (.dmg) ──────────────────────────────────────────────────────
set(CPACK_DMG_VOLUME_NAME "Cmdx ${PROJECT_VERSION}")

# ── Generator selection per platform ─────────────────────────────────────────────
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
endif()

include(CPack)
