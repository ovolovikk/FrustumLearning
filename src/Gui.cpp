#include "Gui.hpp"

#include <Metal/Metal.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_metal.h>

void initGui(GLFWwindow* window, MTL::Device* device)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplMetal_Init(device);
}

void shutdownGui()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void drawGui(MTL::RenderPassDescriptor* pass, MTL::CommandBuffer* commands,
             MTL::RenderCommandEncoder* encoder)
{
    ImGui_ImplMetal_NewFrame(pass);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Frustum Learning");
    ImGui::Text("Device: %s", commands->device()->name()->utf8String());
    ImGui::Text("%.1f fps (%.2f ms)", ImGui::GetIO().Framerate,
                1000.0f / ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commands, encoder);
}
