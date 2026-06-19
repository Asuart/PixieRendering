#include <GLFW/glfw3.h>

#include <PixieRendering/PixieRendering.h>

int32_t main(void) {
    MainWindow* mainWindow = CreateMainWindow("Simple Scene", { 1280, 720 }, RenderAPI::OpenGL);
    Renderer* renderer = CreateRenderer(mainWindow);

    while (!mainWindow->GetShouldClose()) {
        renderer->StartFrame();

        renderer->ClearFrameBuffer();

        renderer->EndFrame();

        mainWindow->SwapBuffers();
        glfwPollEvents();
    }

    return 0;
}
