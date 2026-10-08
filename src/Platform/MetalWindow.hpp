#pragma once

struct GLFWwindow;

namespace CA
{
    class MetalLayer;
}

// The only Objective-C in the project: GLFW gives an NSWindow, Metal wants its
// view to be backed by a CAMetalLayer
void attachMetalLayer(GLFWwindow* window, CA::MetalLayer* layer);
