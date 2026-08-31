#include <filesystem>
#include <iostream>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <PixieRendering/PixieRendering.h>
#include <PixieRendering/Resources/Camera.h>

#include <PixieUIApplication/PixieUIApplication.h>
#include <PixieUIApplication/Windows/DemoWindow.h>
#include <PixieUIApplication/Windows/TextureDisplayWindow.h>

#include "LoadScene.h"

using namespace PixieRenderer;

const char* vertexShaderSource = R"(
#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 boneIDs; 
layout(location = 4) in vec4 boneWeights; 

layout(location = 0) out vec2 TexCoord;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 projection;
} camera;

void main()
{
    gl_Position = camera.projection * camera.view * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 450

layout(location = 0) in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
}
)";

class SimplaeSceneApp : public PixieApp::PixieUIApplication {
  public:
	SimplaeSceneApp(const char* path) : PixieUIApplication("Simple scene", { 1280, 720 }, RenderAPI::Vulkan, true) {
		m_frameBuffer = m_renderer->CreateFrameBuffer({ 1280, 720 }, TextureFormat::RGBA32f);

		m_ui->AddWindow(new PixieUI::DemoWindow(m_renderer));
		m_ui->AddWindow(new PixieUI::TextureDisplayWindow(m_renderer, m_frameBuffer));

		std::filesystem::path appPath = std::filesystem::path(path);
		const std::string filePath = appPath.parent_path().string() + "/cube/cube.obj";

		Mesh* mesh = LoadMesh(filePath);

		glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -5.0f);
		glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
		m_camera.view = glm::lookAt(cameraPosition, center, glm::vec3(0.0f, 1.0f, 0.0f));

		m_meshHandle = m_renderer->CreateMesh(mesh);

		Material materialInfo{ vertexShaderSource, fragmentShaderSource };
		m_materialHandle = m_renderer->CreateMaterial(&materialInfo);

		delete mesh;
	}

	void BeforeDrawFrame() override {
		m_ui->OnBeforeDrawFrame();

		m_renderer->BeginRenderPass(m_frameBuffer);
		float aspect = static_cast<float>(m_window->GetResolution().x) /
		               m_window->GetResolution().y;
		m_camera.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
		m_renderer->LoadUniformBuffer(m_materialHandle, "CameraUBO", &m_camera, sizeof(Camera));
		m_renderer->DrawMesh(m_meshHandle, m_materialHandle);
		m_renderer->EndRenderPass();
	}

  private:
	FrameBufferHandle m_frameBuffer;
	MaterialHandle m_materialHandle;
	MeshHandle m_meshHandle;
	Camera m_camera;
};

int32_t main(int argc, char** argv) {
	SimplaeSceneApp* app = new SimplaeSceneApp(argv[0]);
	app->Start();

	delete app;

	return 0;
}
