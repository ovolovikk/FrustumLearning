cmake_minimum_required(VERSION 3.25)

project(FrustumLearning LANGUAGES CXX OBJCXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_OBJCXX_STANDARD 20)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

# ---- dependencies ----

# brew install glfw
find_package(glfw3 REQUIRED)

include(FetchContent)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.6
    GIT_SHALLOW    TRUE
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui glm)

# Apple's header only C++ wrapper over Metal, vendored in external/
add_library(metal-cpp INTERFACE)
target_include_directories(metal-cpp SYSTEM INTERFACE "${CMAKE_SOURCE_DIR}/external/metal-cpp")
target_link_libraries(metal-cpp INTERFACE
    "-framework Metal"
    "-framework Foundation"
    "-framework QuartzCore"
)

# imgui ships no CMake of its own
add_library(imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm"
)
target_include_directories(imgui SYSTEM PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
)
# Makes the Metal backend accept MTL::Device* and friends instead of ObjC ids
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_METAL_CPP)
# With IMGUI_IMPL_METAL_CPP the backend manages memory by hand, so no ARC.
# Its leftover ARC bridge casts then warn, which is upstream's to fix.
set_source_files_properties("${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm"
    PROPERTIES COMPILE_OPTIONS "-Wno-arc-bridge-casts-disallowed-in-nonarc"
)
target_link_libraries(imgui PUBLIC glfw metal-cpp "-framework Cocoa")

# ---- the app ----

add_executable(FrustumLearning
    src/main.cpp
    src/Gui.cpp
    src/Gui.hpp
    src/Platform/MetalImplementations.cpp
    src/Platform/MetalWindow.hpp
    src/Platform/MetalWindow.mm
)

target_include_directories(FrustumLearning PRIVATE "${CMAKE_SOURCE_DIR}/src")

# Shaders are compiled from source at startup, so they are read straight from the repo
target_compile_definitions(FrustumLearning PRIVATE
    SHADER_DIRECTORY="${CMAKE_SOURCE_DIR}/src/Shaders"
)

target_compile_options(FrustumLearning PRIVATE -Wall -Wextra)

target_link_libraries(FrustumLearning PRIVATE
    glfw
    glm::glm
    imgui
    metal-cpp
)
