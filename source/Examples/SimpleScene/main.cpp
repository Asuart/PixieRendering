#include <GLFW/glfw3.h>

#include <PixieRendering/PixieRendering.h>

using namespace PixieRenderer;

const char* vertexShaderSource = R"(
#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 boneIDs; 
layout(location = 4) in vec4 boneWeights; 

layout(location = 0) out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
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

int32_t main(void) {
	Window* window = CreateWindow("Simple Scene", { 1280, 720 }, RenderAPI::Vulkan);
	IRenderer* renderer = CreateRenderer(window);

	Mesh mesh;
	mesh.vertexes = {
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
		{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
	};
	mesh.indexes = { 0, 1, 2, 0, 2, 3 };

	MeshHandle meshHandle = renderer->CreateMesh(&mesh);

	Material materialInfo{ vertexShaderSource, fragmentShaderSource };
	MaterialHandle materialHandle = renderer->CreateMaterial(&materialInfo);

	while (!window->GetShouldClose()) {
		renderer->StartFrame();

		renderer->DrawMesh(meshHandle, materialHandle);

		renderer->EndFrame();

		window->SwapBuffers();
		window->PollEvents();
	}

	return 0;
}
