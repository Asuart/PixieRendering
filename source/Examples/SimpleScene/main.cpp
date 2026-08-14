#include <GLFW/glfw3.h>

#include <PixieRendering/PixieRendering.h>

int32_t main(void) {
    Window* mainWindow = CreateMainWindow("Simple Scene", { 1280, 720 }, RenderAPI::Vulkan);
    IRenderer* renderer = CreateRenderer(mainWindow);

    while (!mainWindow->GetShouldClose()) {
        renderer->StartFrame();

        renderer->EndFrame();

        mainWindow->SwapBuffers();
        mainWindow->PollEvents();
    }

    return 0;
}
