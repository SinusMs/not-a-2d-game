cmake_minimum_required(VERSION 3.28)
project(NotA2DGame)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories("lib")


# CPM
file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

set(LIBS)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## Dependencies

# ImGui
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.92.1
    DOWNLOAD_ONLY
)
if (imgui_ADDED)
    add_library(imgui STATIC)
    target_sources(imgui PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        # SDL 3
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        # SDL 3 Renderer
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    )
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}/backends")
    list(APPEND LIBS imgui)
endif ()

# SDL
CPMAddPackage("gh:libsdl-org/SDL#release-3.2.18")
list(APPEND LIBS SDL3::SDL3)
if (imgui_ADDED)
    target_link_libraries(imgui PRIVATE SDL3::SDL3)
endif ()

# SDL_image
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE) # Requires Perl and Nasm for libaom (AVIF image format)
CPMAddPackage("gh:libsdl-org/SDL_image#release-3.2.4")
list(APPEND LIBS SDL3_image::SDL3_image)

# flecs
CPMAddPackage("gh:SanderMertens/flecs#v4.1.0")
list(APPEND LIBS flecs::flecs)

# glm
CPMAddPackage("gh:g-truc/glm#1.0.1")
list(APPEND LIBS glm::glm)

# # zlib, tinyxml2 and zstd required by tinytmx
# CPMAddPackage("gh:madler/zlib#v1.3.1")
# list(APPEND LIBS ZLIB::ZLIB)
# CPMAddPackage("gh:leethomason/tinyxml2#11.0.0")
# list(APPEND LIBS tinyxml2::tinyxml2)
# CPMAddPackage("gh:facebook/zstd#v1.5.7")
# list(APPEND LIBS ZLIB::ZLIB)
# CPMAddPackage(
#   NAME zstd
#   GITHUB_REPOSITORY facebook/zstd
#   VERSION 1.5.7
#   OPTIONS "ZSTD_BUILD_PROGRAMS OFF" "ZSTD_BUILD_SHARED OFF"
#   DOWNLOAD_ONLY TRUE  # We’ll build it manually
# )
# # Build zstd manually using its Makefile
# add_custom_command(
#   OUTPUT ${zstd_SOURCE_DIR}/lib/libzstd.a
#   WORKING_DIRECTORY ${zstd_SOURCE_DIR}/build
#   COMMAND ${CMAKE_COMMAND} -E echo "Building zstd..."
#   COMMAND ${CMAKE_MAKE_PROGRAM} libzstd.a
#   DEPENDS ${zstd_SOURCE_DIR}/lib
# )
# add_custom_target(zstd_target
#   DEPENDS ${zstd_SOURCE_DIR}/lib/libzstd.a
# )
# # Create an interface target for linking
# add_library(zstd::zstd STATIC IMPORTED GLOBAL)
# set_target_properties(zstd::zstd PROPERTIES
#   IMPORTED_LOCATION ${zstd_SOURCE_DIR}/lib/libzstd.a
#   INTERFACE_INCLUDE_DIRECTORIES ${zstd_SOURCE_DIR}/lib
# )
# add_dependencies(zstd::zstd zstd_target)
# list(APPEND LIBS zstd::zstd)
# # tinytmx
# CPMAddPackage("gh:KaseyJenkins/tinytmx#ef0d4e1")
# list(APPEND LIBS tinytmx::tinytmx)

# SQLite
# CPMAddPackage("gh:SRombauts/SQLiteCpp#643b153")
# list(APPEND LIBS SQLiteCpp)


## Executables
add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

file(COPY Content DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})