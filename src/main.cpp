#include <algorithm>
#include <cstdio>
#include <fstream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <GLFW/glfw3.h>

#include "Gui.hpp"
#include "Platform/MetalWindow.hpp"

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr MTL::PixelFormat COLOR_FORMAT = MTL::PixelFormatBGRA8Unorm;
constexpr const char* SHADER_PATH = SHADER_DIRECTORY "/Shaders.metal";

void logError(const char* what, NS::Error* error)
{
    const char* reason = error ? error->localizedDescription()->utf8String() : "unknown";
    std::fprintf(stderr, "[Metal] %s: %s\n", what, reason);
}

NS::SharedPtr<MTL::Function> loadFunction(MTL::Library* library, const char* name)
{
    return NS::TransferPtr(library->newFunction(NS::String::string(name, NS::UTF8StringEncoding)));
}

NS::SharedPtr<MTL::Library> compileShaders(MTL::Device* device, const char* path)
{
    std::ifstream file(path);
    if (!file)
    {
        std::fprintf(stderr, "[Metal] cannot open shader file: %s\n", path);
        return {};
    }
    std::stringstream source;
    source << file.rdbuf();

    NS::Error* error = nullptr;
    auto library = NS::TransferPtr(device->newLibrary(
        NS::String::string(source.str().c_str(), NS::UTF8StringEncoding), nullptr, &error));
    if (!library)
    {
        logError("shader compilation failed", error);
    }
    return library;
}

NS::SharedPtr<MTL::RenderPipelineState> createTrianglePipeline(MTL::Device* device,
                                                               MTL::Library* library)
{
    auto vertexFunction = loadFunction(library, "vertexMain");
    auto fragmentFunction = loadFunction(library, "fragmentMain");

    auto descriptor = NS::TransferPtr(MTL::RenderPipelineDescriptor::alloc()->init());
    descriptor->setVertexFunction(vertexFunction.get());
    descriptor->setFragmentFunction(fragmentFunction.get());
    descriptor->colorAttachments()->object(0)->setPixelFormat(COLOR_FORMAT);

    NS::Error* error = nullptr;
    auto pipeline = NS::TransferPtr(device->newRenderPipelineState(descriptor.get(), &error));
    if (!pipeline)
    {
        logError("render pipeline creation failed", error);
    }
    return pipeline;
}

// Runs a kernel over a buffer and checks the result on the CPU, so the
// compute path is known to work before any GPU frustum code relies on it
bool runComputeSelfTest(MTL::Device* device, MTL::CommandQueue* queue, MTL::Library* library)
{
    constexpr size_t VALUE_COUNT = 1024;

    auto function = loadFunction(library, "doubleValues");

    NS::Error* error = nullptr;
    auto pipeline = NS::TransferPtr(device->newComputePipelineState(function.get(), &error));
    if (!pipeline)
    {
        logError("compute pipeline creation failed", error);
        return false;
    }

    std::vector<float> values(VALUE_COUNT);
    std::iota(values.begin(), values.end(), 0.0f);

    auto buffer = NS::TransferPtr(device->newBuffer(
        values.data(), values.size() * sizeof(float), MTL::ResourceStorageModeShared));

    MTL::CommandBuffer* commands = queue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = commands->computeCommandEncoder();
    encoder->setComputePipelineState(pipeline.get());
    encoder->setBuffer(buffer.get(), 0, 0);
    encoder->dispatchThreads(
        MTL::Size(VALUE_COUNT, 1, 1),
        MTL::Size(std::min<size_t>(VALUE_COUNT, pipeline->maxTotalThreadsPerThreadgroup()), 1, 1));
    encoder->endEncoding();
    commands->commit();
    commands->waitUntilCompleted();

    const float* result = static_cast<const float*>(buffer->contents());
    for (size_t i = 0; i < VALUE_COUNT; ++i)
    {
        if (result[i] != values[i] * 2.0f)
        {
            std::fprintf(stderr, "[SelfTest] value %zu: expected %f, got %f\n", i,
                         values[i] * 2.0f, result[i]);
            return false;
        }
    }
    return true;
}

int main()
{
    if (!glfwInit())
    {
        std::fprintf(stderr, "[GLFW] initialization failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window =
        glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Frustum Learning", nullptr, nullptr);
    if (!window)
    {
        std::fprintf(stderr, "[GLFW] window creation failed\n");
        glfwTerminate();
        return 1;
    }

    auto device = NS::TransferPtr(MTL::CreateSystemDefaultDevice());
    if (!device)
    {
        std::fprintf(stderr, "[Metal] no device\n");
        return 1;
    }
    auto queue = NS::TransferPtr(device->newCommandQueue());

    CA::MetalLayer* layer = CA::MetalLayer::layer();
    layer->setDevice(device.get());
    layer->setPixelFormat(COLOR_FORMAT);
    attachMetalLayer(window, layer);

    auto library = compileShaders(device.get(), SHADER_PATH);
    if (!library)
    {
        return 1;
    }
    auto trianglePipeline = createTrianglePipeline(device.get(), library.get());
    if (!trianglePipeline)
    {
        return 1;
    }

    const bool computeWorks = runComputeSelfTest(device.get(), queue.get(), library.get());
    std::printf("[SelfTest] device: %s\n", device->name()->utf8String());
    std::printf("[SelfTest] compute kernel: %s\n", computeWorks ? "PASS" : "FAIL");
    std::fflush(stdout);

    initGui(window, device.get());

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        NS::AutoreleasePool* framePool = NS::AutoreleasePool::alloc()->init();

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        layer->setDrawableSize(CGSizeMake(width, height));

        CA::MetalDrawable* drawable = layer->nextDrawable();
        if (!drawable)
        {
            framePool->release();
            continue;
        }

        MTL::RenderPassDescriptor* pass = MTL::RenderPassDescriptor::renderPassDescriptor();
        MTL::RenderPassColorAttachmentDescriptor* color = pass->colorAttachments()->object(0);
        color->setTexture(drawable->texture());
        color->setLoadAction(MTL::LoadActionClear);
        color->setStoreAction(MTL::StoreActionStore);
        color->setClearColor(MTL::ClearColor(0.08, 0.09, 0.12, 1.0));

        MTL::CommandBuffer* commands = queue->commandBuffer();
        MTL::RenderCommandEncoder* encoder = commands->renderCommandEncoder(pass);

        encoder->setRenderPipelineState(trianglePipeline.get());
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));

        drawGui(pass, commands, encoder);

        encoder->endEncoding();
        commands->presentDrawable(drawable);
        commands->commit();

        framePool->release();
    }

    shutdownGui();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
