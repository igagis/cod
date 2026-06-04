cmake_minimum_required(VERSION 3.30)

project(cod)

# !!! find_package must go after project() declaration !!!
# Otherwise VCPKG does not set the CMAKE_PREFIX_PATH to find packages.
find_package(myci CONFIG REQUIRED)

# ===================================================
# == declare cod as a library to be unit testable ===
set(lib_srcs)
myci_add_source_files(lib_srcs
    DIRECTORY
        ../../src/lib
    RECURSIVE
)

myci_declare_library(${PROJECT_NAME}
    SOURCES
        ${lib_srcs}
    PUBLIC_INCLUDE_DIRECTORIES
        ../../src/lib/src
)

# ==================================
# === declare text_editor plugin ===

set(text_editor_srcs)
myci_add_source_files(text_editor_srcs
    DIRECTORY
        ../../src/plugins/text_editor
    RECURSIVE
)

myci_declare_library(text-editor
    SOURCES
        ${text_editor_srcs}
    DEPENDENCIES
        ${PROJECT_NAME}::${PROJECT_NAME}
)

# =====================================
# === declare cod as an application ===

set(app_srcs)
myci_add_source_files(app_srcs
    DIRECTORY
        ../../src/cod
    RECURSIVE
)

myci_declare_application(${PROJECT_NAME}-opengl
    SOURCES
        ${app_srcs}
    RESOURCE_DIRECTORY
        ../../res
    DEPENDENCIES
        clargs
        ruisapp::ruisapp-opengl
        ${PROJECT_NAME}::${PROJECT_NAME}
        ${PROJECT_NAME}::text-editor
    # RUN_ARGUMENTS
    #     --res-path=res
    #     --window
)
