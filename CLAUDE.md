# FrustumLearning

A learning project: several frustum culling implementations in C++ and Metal,
compared against each other on the same scene.

## Who writes what

AI is used only for boilerplate: build setup, windowing, Metal plumbing, ImGui,
file layout, refactors. The culling algorithms are written by the user.

- Do not write or complete a culling algorithm, on the CPU or in a shader,
  unless the user explicitly asks for that specific piece.
- Explaining concepts, reviewing the user's code and pointing out bugs is fine.
  Prefer explaining over handing over a finished implementation.

## Planned methods

- CPU: AABB against the frustum planes
- GPU: compute kernel culling into an indirect draw

## Build and run

    cmake -S . -B build
    cmake --build build
    ./build/FrustumLearning

Needs `brew install glfw`. imgui and glm come through FetchContent, metal-cpp is
vendored in `external/`.

## Layout

- `src/main.cpp`: window, device, frame loop
- `src/Gui.cpp`: all ImGui code, the user does not want it anywhere else
- `src/Shaders/`: `.metal` sources, read from disk and compiled at startup
- `src/Platform/`: the metal-cpp implementation unit and the only Objective-C++ file

## Code style

- Keep it simple: free functions, no anonymous namespaces
- Allman braces, 4 spaces, `UPPER_CASE` constants 
