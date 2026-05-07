cmake_minimum_required(VERSION 3.16)
project(cmdx VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(nlohmann_json REQUIRED)

set(CMDX_MAC_CODESIGN_IDENTITY "" CACHE STRING "Developer ID Application identity used to sign macOS binaries")
set(CMDX_MAC_INSTALLER_IDENTITY "" CACHE STRING "Developer ID Installer identity used to sign macOS packages")
set(CMDX_MAC_KEYCHAIN_PATH "" CACHE FILEPATH "Keychain path containing Apple signing identities")

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

if(APPLE)
    set_target_properties(cmdx PROPERTIES MACOSX_BUNDLE TRUE)
endif()

target_compile_definitions(cmdx PRIVATE
    CMDX_VERSION="${PROJECT_VERSION}"
    CMDX_DATA_DIR="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/cmdx"
)

target_include_directories(cmdx PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(cmdx PRIVATE Qt6::Widgets nlohmann_json::nlohmann_json)

# Dev convenience: copy commands into build dir for running in-tree
file(COPY ${CMAKE_SOURCE_DIR}/commands DESTINATION ${CMAKE_BINARY_DIR})

# ─── Install ────────────────────────────────────────────────────────────────────

if(APPLE)
    # Install the .app bundle at the prefix root so macdeployqt finds it
    install(TARGETS cmdx BUNDLE DESTINATION .)

    # Generate a CLI wrapper script: bin/cmdx -> ../cmdx.app/Contents/MacOS/cmdx
    configure_file(
        ${CMAKE_SOURCE_DIR}/packaging/macos/cmdx.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmdx-wrapper
        @ONLY
    )
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cmdx-wrapper
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        RENAME cmdx
    )
else()
    install(TARGETS cmdx RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# Install the full command library
install(DIRECTORY commands/ DESTINATION ${CMAKE_INSTALL_DATADIR}/cmdx/commands)
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/cmdx)

if(WIN32 OR APPLE)
    set(cmdx_deploy_args
        TARGET cmdx
        OUTPUT_SCRIPT cmdx_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )

    if(WIN32)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
        set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
        include(InstallRequiredSystemLibraries)
        list(APPEND cmdx_deploy_args NO_COMPILER_RUNTIME)
    elseif(APPLE AND CMDX_MAC_CODESIGN_IDENTITY)
        list(APPEND cmdx_deploy_args
            DEPLOY_TOOL_OPTIONS
                "-sign-for-notarization=${CMDX_MAC_CODESIGN_IDENTITY}"
        )
    endif()

    qt_generate_deploy_app_script(${cmdx_deploy_args})
    install(SCRIPT ${cmdx_deploy_script})
endif()

# ─── CPack ──────────────────────────────────────────────────────────────────────

set(CPACK_PACKAGE_NAME "cmdx")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multiplatform command reference and execution console")
set(CPACK_PACKAGE_DESCRIPTION "Cmdx is a GUI and terminal command builder. It reads a JSON database of CLI tools and presents an interactive form to construct commands with proper flags, arguments, and validation.")
set(CPACK_PACKAGE_VENDOR "cmdx")
set(CPACK_PACKAGE_CONTACT "snatcha <snatcha@cmdx.dev>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/packaging/LICENSE.txt")
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
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Cmdx")
set(CPACK_NSIS_DISPLAY_NAME "Cmdx")
set(CPACK_NSIS_PACKAGE_NAME "Cmdx ${PROJECT_VERSION}")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ${CMAKE_INSTALL_BINDIR})
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_HELP_LINK "https://github.com/Ishant89op/cmdx")
set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/Ishant89op/cmdx")
set(CPACK_NSIS_CONTACT "snatcha@cmdx.dev")

# ── Generator selection per platform ─────────────────────────────────────────────
if(WIN32)
    set(CPACK_PACKAGE_FILE_NAME "Cmdx-Setup-${PROJECT_VERSION}-win64")
    set(CPACK_GENERATOR "NSIS")
elseif(APPLE)
    set(CPACK_PACKAGE_FILE_NAME "Cmdx-Installer-${PROJECT_VERSION}-macos")
    set(CPACK_PRODUCTBUILD_IDENTIFIER "dev.cmdx.pkg")
    set(CPACK_PRODUCTBUILD_DOMAINS TRUE)
    set(CPACK_PRODUCTBUILD_DOMAINS_ANYWHERE FALSE)
    set(CPACK_PRODUCTBUILD_DOMAINS_USER FALSE)
    set(CPACK_PRODUCTBUILD_DOMAINS_ROOT TRUE)
    if(CMDX_MAC_INSTALLER_IDENTITY)
        set(CPACK_PRODUCTBUILD_IDENTITY_NAME "${CMDX_MAC_INSTALLER_IDENTITY}")
        set(CPACK_PKGBUILD_IDENTITY_NAME "${CMDX_MAC_INSTALLER_IDENTITY}")
    endif()
    if(CMDX_MAC_KEYCHAIN_PATH)
        set(CPACK_PRODUCTBUILD_KEYCHAIN_PATH "${CMDX_MAC_KEYCHAIN_PATH}")
        set(CPACK_PKGBUILD_KEYCHAIN_PATH "${CMDX_MAC_KEYCHAIN_PATH}")
    endif()
    set(CPACK_GENERATOR "productbuild")
else()
    set(cmdx_linux_generator "DEB")
    if(EXISTS "/etc/os-release")
        file(READ "/etc/os-release" cmdx_os_release)
        string(TOLOWER "${cmdx_os_release}" cmdx_os_release)
        if(cmdx_os_release MATCHES "(^|\\n)id(_like)?=.*(rhel|fedora|centos|rocky|alma|suse)")
            set(cmdx_linux_generator "RPM")
        endif()
    endif()
    set(CPACK_GENERATOR "${cmdx_linux_generator}")
endif()

include(CPack)
