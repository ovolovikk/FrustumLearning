#include "Platform/MetalWindow.hpp"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

void attachMetalLayer(GLFWwindow* window, CA::MetalLayer* layer)
{
    NSWindow* nsWindow = glfwGetCocoaWindow(window);
    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;

    metalLayer.contentsScale = nsWindow.backingScaleFactor;
    nsWindow.contentView.layer = metalLayer;
    nsWindow.contentView.wantsLayer = YES;
}
