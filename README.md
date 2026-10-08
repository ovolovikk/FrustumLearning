# FrustumLearning

Learning project: frustum culling implementations in C++ and Metal, compared on
the same scene.

## Methods

### CPU: AABB against frustum planes

TODO

### GPU: compute culling with indirect draw

TODO

## Build

macOS only.

    brew install glfw
    cmake -S . -B build
    cmake --build build
    ./build/FrustumLearning

## License

MIT, see [LICENSE](LICENSE). metal-cpp in `external/` is Apple's, under Apache 2.0.
