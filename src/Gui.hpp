#pragma once

struct GLFWwindow;

namespace MTL
{
    class Device;
    class CommandBuffer;
    class RenderCommandEncoder;
    class RenderPassDescriptor;
}

void initGui(GLFWwindow* window, MTL::Device* device);
void shutdownGui();

// Builds the whole imgui frame and records its draw calls into the encoder
void drawGui(MTL::RenderPassDescriptor* pass, MTL::CommandBuffer* commands,
             MTL::RenderCommandEncoder* encoder);
