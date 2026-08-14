#include <GLFW/glfw3.h>

#include <PixieRendering/PixieRendering.h>

using namespace PixieRenderer;

const char* vertexShaderSource = R"(
#version 450

// Входные атрибуты вершины (позиция и UV)
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;   // Не используется, но объявлен под вашу структуру
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 boneIDs; 
layout(location = 4) in vec4 bondeWeights; 

// Выход в фрагментный шейдер
layout(location = 0) out vec2 TexCoord;

void main()
{
    // Просто передаем координаты напрямую на экран (без матриц)
    gl_Position = vec4(aPos, 1.0);
    
    // Передаем UV дальше
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 450

// Вход из вершинного шейдера
layout(location = 0) in vec2 TexCoord;

// Выходной цвет пикселя
layout(location = 0) out vec4 FragColor;

void main()
{
    // Вместо текстуры красим треугольник, используя UV как цвета (Красный и Зеленый)
    FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
}
)";

int32_t main(void) {
	Window* window = CreateWindow("Simple Scene", { 1280, 720 }, RenderAPI::Vulkan);
	IRenderer* renderer = CreateRenderer(window);

	Mesh mesh;
	mesh.m_positions = {
		glm::vec3(-0.5f, -0.5f, 0.0f),
		glm::vec3(0.5f, -0.5f, 0.0f),
		glm::vec3(0.0f, 0.5f, 0.0f),
	};
	mesh.m_normals = {
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
	};
	mesh.m_texCoords = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(0.5f, 1.0f),
		glm::vec2(1.0f, 0.0f),
	};

	MeshHandle meshHandle = renderer->CreateMesh(&mesh);

	MaterialHandle materialHandle =
	    renderer->CreateMaterial(vertexShaderSource, fragmentShaderSource);

	while (!window->GetShouldClose()) {
		renderer->StartFrame();

		renderer->DrawMesh(meshHandle, materialHandle);

		renderer->EndFrame();

		window->SwapBuffers();
		window->PollEvents();
	}

	return 0;
}
