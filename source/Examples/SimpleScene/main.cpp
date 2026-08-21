#include <filesystem>
#include <iostream>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <PixieRendering/PixieRendering.h>
#include <PixieRendering/Resources/Camera.h>

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

int32_t main(int argc, char** argv) {
	std::filesystem::path appPath = std::filesystem::path(argv[0]);
	const std::string filePath = appPath.parent_path().string() + "/cube/cube.obj";

	Window* window = CreateWindow("Simple Scene", { 1280, 720 }, RenderAPI::Vulkan);
	IRenderer* renderer = CreateRenderer(window);

	Mesh* mesh = LoadMesh(filePath);

	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -5.0f);
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
	Camera camera;
	camera.view = glm::lookAt(cameraPosition, center, glm::vec3(0.0f, 1.0f, 0.0f));
	camera.projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

	// Mesh mesh;
	// mesh.vertexes = {
	//	{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
	//	{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
	//	{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
	//	{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
	// };
	// mesh.indexes = { 0, 1, 2, 0, 2, 3 };

	MeshHandle meshHandle = renderer->CreateMesh(mesh);

	Material materialInfo{ vertexShaderSource, fragmentShaderSource };
	MaterialHandle materialHandle = renderer->CreateMaterial(&materialInfo);

	while (!window->GetShouldClose()) {
		renderer->StartFrame();

		renderer->LoadUniformBuffer(materialHandle, "CameraUBO", &camera, sizeof(Camera));
		renderer->DrawMesh(meshHandle, materialHandle);

		renderer->EndFrame();

		window->SwapBuffers();
		window->PollEvents();
	}

	delete mesh;

	return 0;
}
